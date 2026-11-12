#include "station.hpp"

#include <Arduino.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <SPI.h>

#include "sm_namespace.hpp"

BEGIN_SPORTMESH_NAMESPACE

#define CHECK_OR_RETURN_ERROR(expr)   \
  if ((err = (expr)) != ErrCode::kOk) \
    return err;

#define IS_ADDR_VALID(addr) \
  (addr != kInvalidAddress)

// valid adresses in range 0001..7FFF
static constexpr lm::AddressType kInvalidAddress = 0;

static constexpr const char* kTag = "station";

static constexpr uint32_t kBaseStDiscoverInt_ms     = 3000;
static constexpr uint32_t kReaderResponseTimeout_ms = 100;

static constexpr uint32_t kReadQueueLen = 20;

void taskDiscoverBaseStation(void* param);

// Triggered by reader
void taskRegisterPunch(void* param);


Station::Station(lm::PinConfig pin_config, 
                 lm::RadioConfig radio_config,
                 StationType station_type)

  : m_base_station_addr{ kInvalidAddress },
    m_register_task_handle{ nullptr },
    m_read_queue{ nullptr },
    m_type{ station_type },
    m_is_lora_running{ false }
{

  lm::LoRaMeshProtocolConfig protocol_config;
  protocol_config.setNodeCapabilities(station_type);
  
  if (m_type == kBaseStation)
    protocol_config.setNodeRole(lm::NodeRole::NETWORK_MANAGER);
  else
    protocol_config.setNodeRole(lm::NodeRole::NODE_ONLY);
  
  m_mesher = lm::LoraMesher::Builder()
                 .withRadioConfig(radio_config)
                 .withPinConfig(pin_config)
                 .withLoRaMeshProtocol(protocol_config)
                 .Build();

  m_mesher->SetDataCallback(
    [this](lm::AddressType source, const std::vector<uint8_t>& data) {
      DataCallback(source, data); 
    }
  );
  
}

ErrCode Station::Start()
{
  if (!m_is_lora_running) {
    lm::Result init_result = m_mesher->Start();
    if (!init_result) {
      ESP_LOGE("station", "%s", init_result.GetErrorMessage().c_str());
      return ErrCode::kLoraMesher;
    }
    m_is_lora_running = true;
  }

  ErrCode err = ErrCode::kOk;

  if (m_type == kCheckpoint || m_type == kRetranslator) {
    CHECK_OR_RETURN_ERROR(CreateBaseStationDiscoverTask());
  }
  
  if (m_type == kCheckpoint) {
    CHECK_OR_RETURN_ERROR(CreateRegisterPunshTask());
  }

  return ErrCode::kOk;
}

ErrCode Station::ChangeType(StationType new_type)
{
  if (new_type == m_type) return ErrCode::kOk;

  ErrCode err = ErrCode::kOk;
  lm::Result res{};

  res = m_mesher->setNodeCapabilities(new_type);
  if (!res) {
    ESP_LOGE(kTag, "Failed to change node type");
    return ErrCode::kLoraMesher;
  }

  switch (m_type) {
    case kBaseStation:
      res = m_mesher->setNodeRole(lm::NodeRole::NODE_ONLY);
      if (!res) {
        ESP_LOGE(kTag, "Failed to change node role");
        return ErrCode::kLoraMesher;
      }

      CHECK_OR_RETURN_ERROR(CreateBaseStationDiscoverTask());

      if (new_type == kCheckpoint)
        CHECK_OR_RETURN_ERROR(CreateRegisterPunshTask());
      
      break;

    case kCheckpoint:
      if (new_type == kBaseStation) {
        res = m_mesher->setNodeRole(lm::NodeRole::NODE_ONLY);
        if (!res) {
          ESP_LOGE(kTag, "Failed to change node role");
          return ErrCode::kLoraMesher;
        }
      }

      CHECK_OR_RETURN_ERROR(DeleteRegisterPunchTask());
      break;

    case kRetranslator:
      if (new_type == kBaseStation) {
        res = m_mesher->setNodeRole(lm::NodeRole::NODE_ONLY);
        if (!res) {
          ESP_LOGE(kTag, "Failed to change node role");
          return ErrCode::kLoraMesher;
        }
      }

      if (new_type == kCheckpoint)
        CHECK_OR_RETURN_ERROR(CreateRegisterPunshTask());
      break;

    default:
      break;
  }
  
  return ErrCode::kOk;
}

ErrCode Station::RegisterPunch(PunchEvent msg)
{
  if (!IS_ADDR_VALID(m_base_station_addr)) {
    ESP_LOGE(kTag, "base station undiscovered");
    return ErrCode::kAddrInvalid;
  }
  
  auto capatabilities = m_mesher->GetNodeCapabilities();
  if (capatabilities & StationType::kBaseStation 
      || capatabilities & StationType::kCheckpoint)
    return ErrCode::kOk;
  
  std::vector<std::uint8_t> data;
  msg.serialize(data);
  lm::Result res = m_mesher->Send(m_base_station_addr, data);

  if (!res) {
    ESP_LOGE(kTag, "%s", res.GetErrorMessage().c_str());
    return ErrCode::kLoraMesher;
  }

  return ErrCode::kOk;
}

ErrCode Station::DiscoverBaseStation()
{
  auto routes = m_mesher->GetRoutingTable();

  if (m_mesher->GetNodeCapabilities() & StationType::kBaseStation)
    return ErrCode::kStationTypeMismatch;

  if (routes.empty()) {
    ESP_LOGW(kTag, "No routes in table");
    return ErrCode::kRouteTableEmpty;
  }

  auto base = 
    m_mesher->GetClosestNodeByCapability(StationType::kBaseStation);

  if (!base.has_value()) {
    m_base_station_addr = kInvalidAddress;
    ESP_LOGW(kTag, "No base station in route table");
    return ErrCode::kAddrInvalid;
  }

  m_base_station_addr = base.value().destination;
  ESP_LOGI(kTag, "Discovered base station %x", m_base_station_addr);

  return ErrCode::kOk;
}

void Station::DataCallback(lm::AddressType source, 
                           const std::vector<uint8_t>& data) {

  ESP_LOGI(kTag, "Received data from: 0x%X (%zu bytes)",
           static_cast<unsigned int>(source), data.size());

  if (data.empty()) {
      ESP_LOGI(kTag, "(no data)");
  } else {
      ESP_LOG_BUFFER_HEX_LEVEL(kTag, data.data(), data.size(), ESP_LOG_INFO);
  }
}

ErrCode Station::CreateBaseStationDiscoverTask()
{
  auto res = xTaskCreate(
      taskDiscoverBaseStation, "discover base",
      512, this,
      1, nullptr);

  if (res != pdPASS) {
    ESP_LOGE(kTag, "Failed to create base discover task [RTOS err code %d]", res);
    return ErrCode::kRTOS;
  }
}

ErrCode Station::CreateRegisterPunshTask()
{
  auto res = xTaskCreate(
    taskRegisterPunch, "register punch",
    512, this,
    2, &m_register_task_handle);

  if (res != pdPASS) {
    ESP_LOGE(kTag, "Failed to create punch register task [RTOS err code %d]", res);
    return ErrCode::kRTOS;
  }

  m_read_queue = xQueueCreate(kReadQueueLen, sizeof(PunchEvent));

  if (!m_read_queue) {
    ESP_LOGE(kTag, "Failed to create read queue [RTOS err code %d]", res);
    return ErrCode::kRTOS;
  }
}

ErrCode Station::DeleteRegisterPunchTask()
{
  if (m_register_task_handle != NULL)
    vTaskDelete(m_register_task_handle);

  if (m_read_queue != NULL)
    vQueueDelete(m_read_queue);
}

void taskRegisterPunch(void* param)
{
  Station* station = static_cast<Station*>(param); 
  TickType_t timeout = pdMS_TO_TICKS(kReaderResponseTimeout_ms);
  BaseType_t res = pdFAIL;
  PunchEvent event{};
  
  for (;;) {
    if (ulTaskNotifyTake(pdTRUE, timeout)) {
      res = xQueueReceive(station->m_read_queue, 
                          &event,
                          static_cast<TickType_t>(0));
      if (res != pdPASS) {
        ESP_LOGE(kTag, "Failed to recieve event from queue");
        continue;
      }
      station->RegisterPunch(event);
      ESP_LOGI(kTag, "Registered punch");
    }
  }
}

void taskDiscoverBaseStation(void* param)
{
  auto station = static_cast<Station*>(param);
  const TickType_t interval = pdMS_TO_TICKS(kBaseStDiscoverInt_ms);
  TickType_t last_wake = xTaskGetTickCount();

  while (station->DiscoverBaseStation() != ErrCode::kOk)
    xTaskDelayUntil(&last_wake, interval);

  vTaskDelete(nullptr);
}

END_SPORTMESH_NAMESPACE

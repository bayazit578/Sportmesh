#include "station.hpp"

#include "Arduino.h"
#include "esp_log.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include <SPI.h>

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
    m_read_queue{ nullptr }
{

  lm::LoRaMeshProtocolConfig protocol_config;
  protocol_config.setNodeCapabilities(station_type);
  
  if (station_type == kBaseStation)
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

int Station::Start()
{
  lm::Result init_result = m_mesher->Start();
  if (!init_result) {
    ESP_LOGE("station", "%s", init_result.GetErrorMessage().c_str());
    return 1;
  }

  BaseType_t res;

  res = xTaskCreate(
      taskDiscoverBaseStation, "discover base",
      4096, this,
      1, nullptr);

  if (res != pdPASS) {
    ESP_LOGE(kTag, "Failed to create base discover task");
    return 1;
  }
  
  res = xTaskCreate(
    taskRegisterPunch, "register punch",
    4096, this,
    2, &m_register_task_handle);

  if (res != pdPASS) {
    ESP_LOGE(kTag, "Failed to create punch register task");
    return 1;
  }

  m_read_queue = xQueueCreate(kReadQueueLen, sizeof(PunchEvent));

  if (!m_read_queue) {
    ESP_LOGE(kTag, "Failed to create read queue");
    return 1;
  }

  return 0;
}

int Station::RegisterPunch(PunchEvent msg)
{
  if (!IS_ADDR_VALID(m_base_station_addr)) {
    ESP_LOGW(kTag, "base station undiscovered");
    return 1;
  }
  
  auto capatabilities = m_mesher->GetNodeCapabilities();
  if (capatabilities & StationType::kBaseStation 
      || capatabilities & StationType::kCheckpoint)
    return 0;
  
  std::vector<uint8_t> data;
  msg.serialize(data);
  lm::Result res = m_mesher->Send(m_base_station_addr, data);

  if (!res) {
    ESP_LOGE(kTag, "%s", res.GetErrorMessage().c_str());
    return 1;
  }

  return 0;
}

int Station::DiscoverBaseStation()
{
  auto routes = m_mesher->GetRoutingTable();

  if (m_mesher->GetNodeCapabilities() & StationType::kBaseStation)
    return 1;

  if (routes.empty()) {
    ESP_LOGW(kTag, "No routes in table");
    return 1;
  }

  auto base = 
    m_mesher->GetClosestNodeByCapability(StationType::kBaseStation);

  if (!base.has_value()) {
    m_base_station_addr = kInvalidAddress;
    ESP_LOGW(kTag, "No base station in route table");
    return 1;
  }

  m_base_station_addr = base.value().destination;
  ESP_LOGI(kTag, "Discovered base station %x", m_base_station_addr);

  return 0;
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

  while (station->DiscoverBaseStation())
    xTaskDelayUntil(&last_wake, interval);

  vTaskDelete(nullptr);
}

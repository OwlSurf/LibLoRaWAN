cmake_minimum_required(VERSION 3.14)

project(LibLoRaWAN C)

add_library(LoRaWAN STATIC
	Src/LoRa.c
	lib/ideetron/AES-128_V10.c
	lib/ideetron/Encrypt_V31.c
)
target_include_directories(LoRaWAN PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Inc>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/ideetron>
)
set_target_properties(LoRaWAN PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON)

set(RFM95_PATH "" CACHE PATH "Path to OwlSurf/rfm95 (directory that contains rfm95.h or Inc/rfm95.h)")

if(RFM95_PATH)
	find_path(RFM95_INCLUDE_DIR
		NAMES rfm95.h
		PATHS
			"${RFM95_PATH}"
			"${RFM95_PATH}/Inc"
			"${RFM95_PATH}/include"
		NO_DEFAULT_PATH
	)
	if(NOT RFM95_INCLUDE_DIR)
		message(FATAL_ERROR "RFM95_PATH=${RFM95_PATH} was set, but rfm95.h was not found")
	endif()

	add_library(LoRaWAN_rfm95 STATIC
		Src/lora_rfm95.c
	)
	target_link_libraries(LoRaWAN_rfm95 PUBLIC LoRaWAN)
	target_include_directories(LoRaWAN_rfm95 PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Inc>
		"${RFM95_INCLUDE_DIR}"
	)
	set_target_properties(LoRaWAN_rfm95 PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON)
endif()

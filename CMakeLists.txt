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

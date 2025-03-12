#pragma once

#include <Windows.h>
#include <pdh.h>
#include <string>
#include <iostream>
#pragma comment(lib, "pdh.lib")

namespace ResourceMonitor {
	class ResourceMonitor1 {
	private:
		PDH_HQUERY gpuQuery = nullptr;
		PDH_HCOUNTER gpuUsageCounter = nullptr;
		PDH_HCOUNTER gpuMemoryCounter = nullptr;

		PDH_HQUERY cpuQuery = nullptr;
		PDH_HCOUNTER cpuTotal = nullptr;

		PDH_HQUERY query = nullptr;
		PDH_HCOUNTER bytesReceivedCounter = nullptr;
		PDH_HCOUNTER bytesSentCounter = nullptr;


	public:
		ResourceMonitor1() {
			// Initialize CPU Performance Counters

			if (PdhOpenQuery(NULL, 0, &cpuQuery) == ERROR_SUCCESS) {
				PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", 0, &cpuTotal);
				PdhCollectQueryData(cpuQuery);
			}
			else {
				std::cerr << "Failed to initialize CPU counters." << std::endl;
			}

			// Initialize GPU Performance Counters
			if (PdhOpenQuery(NULL, 0, &gpuQuery) == ERROR_SUCCESS) {
				// GPU Engine Utilization
				if (PdhAddEnglishCounter(gpuQuery, L"\\GPU Engine(*)\\Utilization Percentage", 0, &gpuUsageCounter) != ERROR_SUCCESS) {
					std::cerr << "Failed to add GPU Engine counter." << std::endl;
				}

				if (PdhAddEnglishCounter(gpuQuery, L"\\GPU Adapter Memory(*)\\Dedicated Usage", 0, &gpuMemoryCounter) != ERROR_SUCCESS) {
					std::cerr << "Failed to add GPU Memory counter." << std::endl;
				}

				PdhCollectQueryData(gpuQuery);
			}
			else {
				std::cerr << "Failed to initialize GPU counters." << std::endl;
			}

			// Создаем запрос для PDH сети
			if (PdhOpenQuery(nullptr, 0, &query) == ERROR_SUCCESS) {
				// Добавляем счетчик для входящего трафика
				if (PdhAddEnglishCounter(query, L"\\Network Interface(*)\\Bytes Received/sec", 0, &bytesReceivedCounter) != ERROR_SUCCESS) {
					std::cerr << "Failed to add Bytes Received/sec counter." << std::endl;
					bytesReceivedCounter = nullptr;
				}

				// Добавляем счетчик для исходящего трафика
				if (PdhAddEnglishCounter(query, L"\\Network Interface(*)\\Bytes Sent/sec", 0, &bytesSentCounter) != ERROR_SUCCESS) {
					std::cerr << "Failed to add Bytes Sent/sec counter." << std::endl;
					bytesSentCounter = nullptr;
				}

				// Собираем начальные данные
				PdhCollectQueryData(query);
			}
		}

		~ResourceMonitor1() {
			if (cpuQuery) PdhCloseQuery(cpuQuery);
			if (gpuQuery) PdhCloseQuery(gpuQuery);
			if (query) PdhCloseQuery(query);

		}

		double GetCPUUsage() {
			if (!cpuQuery) return 0.0;

			PDH_FMT_COUNTERVALUE counterVal;
			if (PdhCollectQueryData(cpuQuery) == ERROR_SUCCESS &&
				PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal) == ERROR_SUCCESS) {
				return counterVal.doubleValue;
			}

			return 0.0;
		}
		double GetMemoryUsage() {
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(MEMORYSTATUSEX);
			if (GlobalMemoryStatusEx(&memInfo)) {
				return 100.0 * (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / memInfo.ullTotalPhys;
			}
			return 0.0;
		}

		double GetGPUUsage() {
			if (!gpuQuery || !gpuUsageCounter) return 0.0;

			DWORD bufferSize = 0;
			DWORD itemCount = 0;
			PDH_FMT_COUNTERVALUE_ITEM* counterItems = nullptr;

			// Получение размера буфера
			PdhCollectQueryData(gpuQuery);
			PdhGetFormattedCounterArray(gpuUsageCounter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, nullptr);

			// Выделение памяти для массива
			counterItems = (PDH_FMT_COUNTERVALUE_ITEM*)malloc(bufferSize);
			if (!counterItems) {
				std::cerr << "Failed to allocate memory for GPU usage counter items." << std::endl;
				return 0.0;
			}

			// Получение данных
			if (PdhGetFormattedCounterArray(gpuUsageCounter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, counterItems) != ERROR_SUCCESS) {
				free(counterItems);
				std::cerr << "Failed to get formatted counter array for GPU usage." << std::endl;
				return 0.0;
			}

			// Суммируем значения всех графических движков
			double totalUsage = 0.0;
			for (DWORD i = 0; i < itemCount; ++i) {
				totalUsage += counterItems[i].FmtValue.doubleValue;
			}

			// Освобождаем память
			free(counterItems);

			return totalUsage;
		}


		double GetVideoMemoryUsage() {
			if (!gpuQuery || !gpuMemoryCounter) return 0.0;

			DWORD bufferSize = 0;
			DWORD itemCount = 0;
			PDH_FMT_COUNTERVALUE_ITEM* counterItems = nullptr;

			// Получение размера буфера
			PdhCollectQueryData(gpuQuery);
			PdhGetFormattedCounterArray(gpuMemoryCounter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, nullptr);

			// Выделение памяти для массива
			counterItems = (PDH_FMT_COUNTERVALUE_ITEM*)malloc(bufferSize);
			if (!counterItems) {
				std::cerr << "Failed to allocate memory for GPU memory counter items." << std::endl;
				return 0.0;
			}

			// Получение данных
			if (PdhGetFormattedCounterArray(gpuMemoryCounter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, counterItems) != ERROR_SUCCESS) {
				free(counterItems);
				std::cerr << "Failed to get formatted counter array for GPU memory usage." << std::endl;
				return 0.0;
			}

			// Используем первое значение (или обрабатываем массив, если нужно)
			double totalMemoryUsage = 0.0;
			for (DWORD i = 0; i < itemCount; ++i) {
				totalMemoryUsage += counterItems[i].FmtValue.doubleValue; // Суммируем память
			}

			// Освобождаем память
			free(counterItems);

			//double totalMemoryBytes = 8.0 * 1024 * 1024 * 1024; // 8 ГБ (пример)

			totalMemoryUsage = totalMemoryUsage / (1024.0 * 1024.0);

			return totalMemoryUsage; // Возвращаем результат в MB
		}

		double GetTotalBytesReceived() {
			return GetTotalBytes(bytesReceivedCounter) / (1024.0 * 1024.0);
		}

		double GetTotalBytesSent() {
			return GetTotalBytes(bytesSentCounter) / (1024.0 * 1024.0);
		}

private:
	double GetTotalBytes(PDH_HCOUNTER counter) {
		if (!query || !counter) return 0.0;

		DWORD bufferSize = 0;
		DWORD itemCount = 0;
		PDH_FMT_COUNTERVALUE_ITEM* counterItems = nullptr;

		// Собираем данные
		PdhCollectQueryData(query);
		PdhGetFormattedCounterArray(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, nullptr);

		// Выделение памяти для массива
		counterItems = (PDH_FMT_COUNTERVALUE_ITEM*)malloc(bufferSize);
		if (!counterItems) {
			std::cerr << "Failed to allocate memory for counter items." << std::endl;
			return 0.0;
		}

		if (PdhGetFormattedCounterArray(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, counterItems) != ERROR_SUCCESS) {
			free(counterItems);
			std::cerr << "Failed to get formatted counter array." << std::endl;
			return 0.0;
		}

		// Суммируем значения для всех сетевых интерфейсов
		double totalBytesPerSecond = 0.0;
		for (DWORD i = 0; i < itemCount; ++i) {
			totalBytesPerSecond += counterItems[i].FmtValue.doubleValue;
		}

		// Освобождаем память
		free(counterItems);

		return totalBytesPerSecond; // Возвращаем общую скорость в байтах/секунду
	}


	};
}

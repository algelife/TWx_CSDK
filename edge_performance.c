#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <twApi.h>

// ThingWorx連接參數
const char* THINGWORX_HOST = "your_thingworx_host";
const char* THINGWORX_PORT = "your_thingworx_port";
const char* APP_KEY = "your_app_key";
const char* THING_NAME = "your_thing_name";

// 上傳間隔時間（秒）
const int UPLOAD_INTERVAL = 5;

// 獲取CPU使用率
float getCPUUsage() {
    float cpuUsage = 0.0;
    FILE* file = popen("mpstat 1 1 | awk '/Average:/ {print $3}'", "r");
    if (file != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            cpuUsage = atof(buffer);
        }
        pclose(file);
    }
    return cpuUsage;
}

// 獲取記憶體使用率
float getMemoryUsage() {
    float memoryUsage = 0.0;
    FILE* file = popen("free | awk '/Mem:/ {print $3/$2 * 100.0}'", "r");
    if (file != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            memoryUsage = atof(buffer);
        }
        pclose(file);
    }
    return memoryUsage;
}

// 獲取儲存使用率
float getStorageUsage() {
    float storageUsage = 0.0;
    FILE* file = popen("df -h / | awk '/\\// {print $(NF-1)}'", "r");
    if (file != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            buffer[strlen(buffer) - 1] = '\0';
            storageUsage = atof(buffer);
        }
        pclose(file);
    }
    return storageUsage;
}

// 獲取網路使用率
float getNetworkUsage() {
    float networkUsage = 0.0;
    FILE* file = popen("ifstat 1 1 | awk '/Average:/ {print $6}'", "r");
    if (file != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            networkUsage = atof(buffer);
        }
        pclose(file);
    }
    return networkUsage;
}

int main() {
    TW_LOG_LEVEL(TW_LOG_LEVEL_DEBUG);

    // 建立ThingWorx連接
    TW_CONNECTION connection = NULL;
    TW_RESULT result = TWCreateConnection(&connection, TW_PROTOCOL_HTTPS);
    if (result != TW_RESULT_OK) {
        printf("Failed to create ThingWorx connection: %s\n", TW_RESULT_TO_STRING(result));
        return -1;
    }

    // 設定連接參數
    result = TWSetConnectionCredentials(connection, THINGWORX_HOST, THINGWORX_PORT, APP_KEY);
    if (result != TW_RESULT_OK) {
        printf("Failed to set ThingWorx connection credentials: %s\n", TW_RESULT_TO_STRING(result));
        return -1;
    }

    // 連接到ThingWorx
    result = TWConnect(connection);
    if (result != TW_RESULT_OK) {
        printf("Failed to connect to ThingWorx: %s\n", TW_RESULT_TO_STRING(result));
        return -1;
    }

    printf("Connected to ThingWorx\n");

    // 持續上傳資料
    while (1) {
        // 獲取CPU使用率
        float cpuUsage = getCPUUsage();

        // 獲取記憶體使用率
        float memoryUsage = getMemoryUsage();

        // 獲取儲存使用率
        float storageUsage = getStorageUsage();

        // 獲取網路使用率
        float networkUsage = getNetworkUsage();

        // 建立資料值對象
        TW_VALUE* properties = TWCreateValue();
        TWAddNumberPrimitive(properties, "cpuUsage", cpuUsage);
        TWAddNumberPrimitive(properties, "memoryUsage", memoryUsage);
        TWAddNumberPrimitive(properties, "storageUsage", storageUsage);
        TWAddNumberPrimitive(properties, "networkUsage", networkUsage);

        // 寫入屬性到Thing
        result = TWWriteProperties(connection, THING_NAME, properties);
        if (result != TW_RESULT_OK) {
            printf("Failed to write properties: %s\n", TW_RESULT_TO_STRING(result));
        } else {
            printf("Data uploaded to ThingWorx\n");
        }

        // 釋放資料值對象
        TWDeleteValue(properties);

        // 等待上傳間隔
        sleep(UPLOAD_INTERVAL);
    }

    // 關閉連接
    TWCloseConnection(connection);

    return 0;
}

#include <streamlog.hpp>
#ifdef ENABLE_VECTOR_LOGGING
#include <vector>
#endif

#ifdef ENABLE_MAP_LOGGING
#include <map>
#endif

int main()
{
    // Basic data types
    log(TRACE) << "Logging an integer: " << 123;
    log(DEBUG) << "Logging a double: " << 3.14;
    log(INFO) << "Logging a character: " << 'C';
    log(WARN) << "Logging a string: " << "Hello, World!";
    log(ERROR) << "Logging a boolean: " << true;

    //Complex data types
#ifdef ENABLE_VECTOR_LOGGING
    std::vector<int> vec = {1, 2, 3, 4, 5};
    log(TRACE) << "Logging a vector: " << vec;
#endif

#ifdef ENABLE_MAP_LOGGING
    std::map<std::string, int> map = {{"apple", 5}, {"banana", 3}};
    log(DEBUG) << "Logging a map: " << map;
#endif

    // Real-world scenarios
    int itemsProcessed = 10;
    int totalItems = 20;
    log(WARN) << "Processed " << itemsProcessed << " out of " << totalItems << " items.";

    std::string username = "user123";
    log(ERROR) << "Failed to find user with username: " << username;

    std::string serviceName = "Database";
    log(FATAL) << "The " << serviceName << " service failed to start.";

    return 0;
}

#pragma once

#include <aws/dynamodb/DynamoDBClient.h> // DynamoDBClient

// Create DB table if not exists
void createDBTable(const std::string &, const std::string &,
                   const unsigned int &);

void putDBEntry(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                const Aws::String &, const Aws::String &, const char *,
                const char *, const char *, const char *);

void putDBEntry(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                const Aws::String &, const Aws::String &, const char *,
                const char *);

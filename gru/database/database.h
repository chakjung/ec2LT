#pragma once

#include <aws/dynamodb/DynamoDBClient.h> // DynamoDBClient

#include "../availabilityZone/availabilityZone.h" // AZ

// Create DB table if not exists
void createDBTable(const std::string &, const std::string &,
                   const unsigned int &);

void readDBTable(const std::string &, std::vector<AZ *> &);

void putDBEntry(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                const Aws::String &, const Aws::String &, const char *,
                const char *, const char *, const char *);

void putDBEntry(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                const Aws::String &, const Aws::String &, const char *,
                const char *);

void putStatEntry(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                  const Aws::String &, const Aws::String &, Entry &);

#pragma once

#include <aws/dynamodb/DynamoDBClient.h> // DynamoDBClient

// Create DB table if not exists
void createDBTable(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                   const unsigned int &);

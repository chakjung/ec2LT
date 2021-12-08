/**
 * EC2 network latency project
 *
 * File: database.h
 *
 * Author: Jung Chak
 *
 * Manage DynamoDB
 */

#pragma once

#include <aws/dynamodb/DynamoDBClient.h> // DynamoDBClient

#include "../availabilityZone/availabilityZone.h" // AZ

// Create DB table if not exists
void createDBTable(const std::string &, const std::string &,
                   const unsigned int &);

// Read & Cache prior DB entries
void readDBTable(const std::string &, std::vector<AZ *> &);

// Put test entry with RES & HDS
void putDBEntry(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                const Aws::String &, const Aws::String &, const char *,
                const char *, const char *, const char *);

// Put test entry without RES & HDS
void putDBEntry(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                const Aws::String &, const Aws::String &, const char *,
                const char *);

// Put statistic entry to DynamoDB
void putStatEntry(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                  const Aws::String &, const Aws::String &, Entry &);

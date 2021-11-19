#include "database.h"

#include <aws/dynamodb/model/ListTablesRequest.h> // ListTablesRequest

#include <aws/dynamodb/model/CreateTableRequest.h> // CreateTableRequest

#include <aws/dynamodb/model/GlobalSecondaryIndex.h> // GlobalSecondaryIndex

#include <aws/dynamodb/model/DescribeTableRequest.h> // DescribeTableRequest

#include <unistd.h> // sleep

#include "../errorCode.h"
// LISTTABLESERRNUM, CREATETABLEERRNUM, DESCRIBETABLEERRNUM

void createTable(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                 const unsigned int &);

void createStatTable(Aws::DynamoDB::DynamoDBClient &, const std::string &,
                     const unsigned int &);

// Create DB table if not exists
void createDBTable(const std::string &tableName,
                   const std::string &statTableName,
                   const unsigned int &delay) {

  // DataBase Client
  Aws::Client::ClientConfiguration clientConfig;
  Aws::DynamoDB::DynamoDBClient client(clientConfig);

  // List all tables
  Aws::DynamoDB::Model::ListTablesRequest lstReq;
  bool tableExist = false;
  bool statTableExist = false;

  do {
    const Aws::DynamoDB::Model::ListTablesOutcome &lstOutcome =
        client.ListTables(lstReq);
    if (!lstOutcome.IsSuccess()) {
      std::cout << "Failed to list tables\n"
                << lstOutcome.GetError().GetMessage() << std::endl;
      exit(LISTTABLESERRNUM);
    }

    // Foreach table name
    for (const Aws::String &tableNameListed :
         lstOutcome.GetResult().GetTableNames()) {
      // Table name match
      if (tableName == tableNameListed) {
        tableExist = true;
      } else if (statTableName == tableNameListed) {
        statTableExist = true;
      }
      // Both table exists
      if (tableExist && statTableExist) {
        return;
      }
    }

    lstReq.SetExclusiveStartTableName(
        lstOutcome.GetResult().GetLastEvaluatedTableName());
  } while (!lstReq.GetExclusiveStartTableName().empty());

  // No matching table
  if (!tableExist) {
    std::cout << "Creating table " << tableName << " ..." << std::endl;
    createTable(client, tableName, delay);
    std::cout << "Table created\n" << std::endl;
  }

  if (!statTableExist) {
    std::cout << "Creating table " << statTableName << " ..." << std::endl;
    createStatTable(client, statTableName, delay);
    std::cout << "Table created\n" << std::endl;
  }
}

void createTable(Aws::DynamoDB::DynamoDBClient &client,
                 const std::string &tableName, const unsigned int &delay) {

  Aws::DynamoDB::Model::CreateTableRequest crtReq;
  crtReq.SetTableName(tableName);
  crtReq.SetBillingMode(Aws::DynamoDB::Model::BillingMode::PAY_PER_REQUEST);

  Aws::DynamoDB::Model::AttributeDefinition srcHashKey, dstHashKey, utsHashkey;
  srcHashKey.WithAttributeName("SRC").WithAttributeType(
      Aws::DynamoDB::Model::ScalarAttributeType::S);
  dstHashKey.WithAttributeName("DST").WithAttributeType(
      Aws::DynamoDB::Model::ScalarAttributeType::S);
  utsHashkey.WithAttributeName("UTS").WithAttributeType(
      Aws::DynamoDB::Model::ScalarAttributeType::N);
  crtReq.AddAttributeDefinitions(srcHashKey);
  crtReq.AddAttributeDefinitions(dstHashKey);
  crtReq.AddAttributeDefinitions(utsHashkey);

  Aws::DynamoDB::Model::KeySchemaElement srcKSE, dstKSE, utsKSE;
  srcKSE.WithAttributeName("SRC").WithKeyType(
      Aws::DynamoDB::Model::KeyType::HASH);
  dstKSE.WithAttributeName("DST").WithKeyType(
      Aws::DynamoDB::Model::KeyType::HASH);
  utsKSE.WithAttributeName("UTS").WithKeyType(
      Aws::DynamoDB::Model::KeyType::RANGE);

  // Set primary key
  crtReq.AddKeySchema(srcKSE);
  crtReq.AddKeySchema(utsKSE);

  // Set global secondary index
  Aws::DynamoDB::Model::GlobalSecondaryIndex globalSecondaryIndex;
  globalSecondaryIndex.WithIndexName("dstParted");

  Aws::DynamoDB::Model::Projection projection;
  projection.SetProjectionType(Aws::DynamoDB::Model::ProjectionType::ALL);

  globalSecondaryIndex.SetProjection(projection);
  globalSecondaryIndex.AddKeySchema(dstKSE);
  globalSecondaryIndex.AddKeySchema(utsKSE);

  crtReq.AddGlobalSecondaryIndexes(globalSecondaryIndex);

  // Create table
  const Aws::DynamoDB::Model::CreateTableOutcome &crtOutcome =
      client.CreateTable(crtReq);
  if (!crtOutcome.IsSuccess()) {
    std::cout << "Failed to create table " << tableName << "\n"
              << crtOutcome.GetError().GetMessage() << std::endl;
    exit(CREATETABLEERRNUM);
  }

  Aws::DynamoDB::Model::DescribeTableRequest desReq;
  desReq.SetTableName(tableName);

  while (true) {
    const Aws::DynamoDB::Model::DescribeTableOutcome &desOutcome =
        client.DescribeTable(desReq);

    if (!desOutcome.IsSuccess()) {
      std::cout << "Failed to describe table " << tableName << "\n"
                << desOutcome.GetError().GetMessage() << std::endl;
      exit(DESCRIBETABLEERRNUM);
    }

    // Table activated
    if (desOutcome.GetResult().GetTable().GetTableStatus() ==
        Aws::DynamoDB::Model::TableStatus::ACTIVE) {
      break;
    }

    sleep(delay);
  }
}

void createStatTable(Aws::DynamoDB::DynamoDBClient &client,
                     const std::string &statTableName,
                     const unsigned int &delay) {

  Aws::DynamoDB::Model::CreateTableRequest crtReq;
  crtReq.SetTableName(statTableName);
  crtReq.SetBillingMode(Aws::DynamoDB::Model::BillingMode::PAY_PER_REQUEST);

  Aws::DynamoDB::Model::AttributeDefinition srcHashKey, dstHashKey;
  srcHashKey.WithAttributeName("SRC").WithAttributeType(
      Aws::DynamoDB::Model::ScalarAttributeType::S);
  dstHashKey.WithAttributeName("DST").WithAttributeType(
      Aws::DynamoDB::Model::ScalarAttributeType::S);
  crtReq.AddAttributeDefinitions(srcHashKey);
  crtReq.AddAttributeDefinitions(dstHashKey);

  Aws::DynamoDB::Model::KeySchemaElement srcKSE, dstKSE;
  srcKSE.WithAttributeName("SRC").WithKeyType(
      Aws::DynamoDB::Model::KeyType::HASH);
  dstKSE.WithAttributeName("DST").WithKeyType(
      Aws::DynamoDB::Model::KeyType::HASH);

  // Set primary key
  crtReq.AddKeySchema(srcKSE);
  crtReq.AddKeySchema(dstKSE);

  // Create table
  const Aws::DynamoDB::Model::CreateTableOutcome &crtOutcome =
      client.CreateTable(crtReq);
  if (!crtOutcome.IsSuccess()) {
    std::cout << "Failed to create table " << statTableName << "\n"
              << crtOutcome.GetError().GetMessage() << std::endl;
    exit(CREATETABLEERRNUM);
  }

  Aws::DynamoDB::Model::DescribeTableRequest desReq;
  desReq.SetTableName(statTableName);

  while (true) {
    const Aws::DynamoDB::Model::DescribeTableOutcome &desOutcome =
        client.DescribeTable(desReq);

    if (!desOutcome.IsSuccess()) {
      std::cout << "Failed to describe table " << statTableName << "\n"
                << desOutcome.GetError().GetMessage() << std::endl;
      exit(DESCRIBETABLEERRNUM);
    }

    // Table activated
    if (desOutcome.GetResult().GetTable().GetTableStatus() ==
        Aws::DynamoDB::Model::TableStatus::ACTIVE) {
      break;
    }

    sleep(delay);
  }
}

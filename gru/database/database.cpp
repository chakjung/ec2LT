#include "database.h"

#include <aws/dynamodb/model/ListTablesRequest.h> // ListTablesRequest

#include <aws/dynamodb/model/CreateTableRequest.h> // CreateTableRequest

#include <aws/dynamodb/model/GlobalSecondaryIndex.h> // GlobalSecondaryIndex

#include <aws/dynamodb/model/DescribeTableRequest.h> // DescribeTableRequest

#include <aws/dynamodb/model/PutItemRequest.h> // PutItemRequest

#include <unistd.h> // sleep

#include "../errorCode.h" // LISTTABLESERRNUM, CREATETABLEERRNUM, DESCRIBETABLEERRNUM
// PUTRTTENTRYERRNUM

// Create DB table if not exists
void createDBTable(Aws::DynamoDB::DynamoDBClient &client,
                   const std::string &tableName, const unsigned int &delay) {

  // List all tables
  Aws::DynamoDB::Model::ListTablesRequest lstReq;
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
        return;
      }
    }
    lstReq.SetExclusiveStartTableName(
        lstOutcome.GetResult().GetLastEvaluatedTableName());
  } while (!lstReq.GetExclusiveStartTableName().empty());

  // No matching table
  std::cout << "Creating table " << tableName << " ..." << std::endl;

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

  std::cout << "Table created\n" << std::endl;
}

void putRttEntry(Aws::DynamoDB::DynamoDBClient &client,
                 const std::string &tableName, const Aws::String &src,
                 const Aws::String &des, const char *resolveT,
                 const char *handShakeT, const char *uts, const char *rtt) {
  Aws::DynamoDB::Model::PutItemRequest putReq;
  putReq.SetTableName(tableName);

  // Source AZ
  Aws::DynamoDB::Model::AttributeValue source;
  source.SetS(src);
  putReq.AddItem("SRC", source);

  // Destination AZ
  Aws::DynamoDB::Model::AttributeValue destination;
  destination.SetS(des);
  putReq.AddItem("DST", destination);

  // Unix timestamp
  Aws::DynamoDB::Model::AttributeValue unixTimestamp;
  unixTimestamp.SetN(uts);
  putReq.AddItem("UTS", unixTimestamp);

  // DNS resolution time
  Aws::DynamoDB::Model::AttributeValue resolveTime;
  resolveTime.SetN(resolveT);
  putReq.AddItem("RES", resolveTime);

  // TCP handshake time
  Aws::DynamoDB::Model::AttributeValue handshakeTime;
  handshakeTime.SetN(handShakeT);
  putReq.AddItem("HDS", handshakeTime);

  // Round-trip time
  Aws::DynamoDB::Model::AttributeValue roundTripTime;
  roundTripTime.SetN(rtt);
  putReq.AddItem("RTT", roundTripTime);

  const Aws::DynamoDB::Model::PutItemOutcome &putOutcome =
      client.PutItem(putReq);

  if (!putOutcome.IsSuccess()) {
    std::cout << "Failed to put RTT entry\n"
              << putOutcome.GetError().GetMessage() << std::endl;
    exit(PUTRTTENTRYERRNUM);
  }
}

void putRttEntry(Aws::DynamoDB::DynamoDBClient &client,
                 const std::string &tableName, const Aws::String &src,
                 const Aws::String &des, const char *uts, const char *rtt) {
  Aws::DynamoDB::Model::PutItemRequest putReq;
  putReq.SetTableName(tableName);

  // Source AZ
  Aws::DynamoDB::Model::AttributeValue source;
  source.SetS(src);
  putReq.AddItem("SRC", source);

  // Destination AZ
  Aws::DynamoDB::Model::AttributeValue destination;
  destination.SetS(des);
  putReq.AddItem("DST", destination);

  // Unix timestamp
  Aws::DynamoDB::Model::AttributeValue unixTimestamp;
  unixTimestamp.SetN(uts);
  putReq.AddItem("UTS", unixTimestamp);

  // Round-trip time
  Aws::DynamoDB::Model::AttributeValue roundTripTime;
  roundTripTime.SetN(rtt);
  putReq.AddItem("RTT", roundTripTime);

  const Aws::DynamoDB::Model::PutItemOutcome &putOutcome =
      client.PutItem(putReq);

  if (!putOutcome.IsSuccess()) {
    std::cout << "Failed to put RTT entry\n"
              << putOutcome.GetError().GetMessage() << std::endl;
    exit(PUTRTTENTRYERRNUM);
  }
}

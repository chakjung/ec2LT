#include "database.h"

#include <aws/dynamodb/model/QueryRequest.h> // QueryRequest

#include "../errorCode.h" // QUERYTABLEERRNUM

void readDBTable(const std::string &tableName, std::vector<AZ *> &AZs) {
  // DataBase Client
  Aws::Client::ClientConfiguration clientConfig;
  Aws::DynamoDB::DynamoDBClient client(clientConfig);

  for (AZ *&az : AZs) {
    Aws::DynamoDB::Model::QueryRequest queryReq;
    queryReq.SetTableName(tableName);
    queryReq.AddExpressionAttributeValues(
        ":AZId", Aws::DynamoDB::Model::AttributeValue{az->AZId});
    queryReq.SetKeyConditionExpression("SRC = :AZId");

    do {
      const Aws::DynamoDB::Model::QueryOutcome &queryOutcome =
          client.Query(queryReq);
      if (!queryOutcome.IsSuccess()) {
        std::cout << "Failed to query " << az->AZId << "\n"
                  << queryOutcome.GetError().GetMessage() << std::endl;
        exit(QUERYTABLEERRNUM);
      }

      const Aws::Vector<
          Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue>> &items =
          queryOutcome.GetResult().GetItems();

      for (const Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue>
               &item : items) {

        Entry &DSTEntry = az->DST[item.at("DST").GetS()];

        if (item.size() == 6) {
          DSTEntry.AddRES(item.at("RES").GetN());
          DSTEntry.AddHDS(item.at("HDS").GetN());
        }
        DSTEntry.AddRTT(item.at("RTT").GetN());
      }

      queryReq.SetExclusiveStartKey(
          queryOutcome.GetResult().GetLastEvaluatedKey());
    } while (!queryReq.GetExclusiveStartKey().empty());
  }
}

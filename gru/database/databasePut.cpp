#include "database.h"

#include <aws/dynamodb/model/PutItemRequest.h> // PutItemRequest

#include "../errorCode.h" // PUTDBENTRYERRNUM

void putDBEntry(Aws::DynamoDB::DynamoDBClient &client,
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
    std::cout << "Failed to put test entry\n"
              << putOutcome.GetError().GetMessage() << std::endl;
    exit(PUTDBENTRYERRNUM);
  }
}

void putDBEntry(Aws::DynamoDB::DynamoDBClient &client,
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
    std::cout << "Failed to put test entry\n"
              << putOutcome.GetError().GetMessage() << std::endl;
    exit(PUTDBENTRYERRNUM);
  }
}

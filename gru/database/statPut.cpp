/**
 * EC2 network latency project
 *
 * File: statPut.cpp
 *
 * Author: Jung Chak
 *
 * Put statistic result
 */

#include "database.h"

#include <aws/dynamodb/model/PutItemRequest.h> // PutItemRequest

#include "../errorCode.h" // PUTSTATENTRYERRNUM

// Put statistic entry to DynamoDB
void putStatEntry(Aws::DynamoDB::DynamoDBClient &client,
                  const std::string &tableName, const Aws::String &src,
                  const Aws::String &des, Entry &entry) {
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

  // Average RTT
  Aws::DynamoDB::Model::AttributeValue averageRTT;
  averageRTT.SetN(std::to_string(entry.RTTSum / entry.RTT.size()));
  putReq.AddItem("AvgRTT", averageRTT);

  // Max RTT
  Aws::DynamoDB::Model::AttributeValue maxRTT;
  maxRTT.SetN(std::to_string(*(--entry.RTT.end())));
  putReq.AddItem("MaxRTT", maxRTT);

  // Min RTT
  Aws::DynamoDB::Model::AttributeValue minRTT;
  minRTT.SetN(std::to_string(*(entry.RTT.begin())));
  putReq.AddItem("MinRTT", minRTT);

  // RTT percentiles
  Aws::DynamoDB::Model::AttributeValue RTTpercentiles;

  // 25th
  std::shared_ptr<Aws::DynamoDB::Model::AttributeValue> percentile =
      std::make_shared<Aws::DynamoDB::Model::AttributeValue>();
  percentile->SetN(std::to_string(
      *(std::next(entry.RTT.begin(), entry.RTT.size() * 25 / 100))));
  RTTpercentiles.AddLItem(percentile);

  // 50th
  percentile = std::make_shared<Aws::DynamoDB::Model::AttributeValue>();
  percentile->SetN(std::to_string(
      *(std::next(entry.RTT.begin(), entry.RTT.size() * 50 / 100))));
  RTTpercentiles.AddLItem(percentile);

  // 75th
  percentile = std::make_shared<Aws::DynamoDB::Model::AttributeValue>();
  percentile->SetN(std::to_string(
      *(std::next(entry.RTT.begin(), entry.RTT.size() * 75 / 100))));
  RTTpercentiles.AddLItem(percentile);

  // 90th
  percentile = std::make_shared<Aws::DynamoDB::Model::AttributeValue>();
  percentile->SetN(std::to_string(
      *(std::next(entry.RTT.begin(), entry.RTT.size() * 90 / 100))));
  RTTpercentiles.AddLItem(percentile);

  // 99th
  percentile = std::make_shared<Aws::DynamoDB::Model::AttributeValue>();
  percentile->SetN(std::to_string(
      *(std::next(entry.RTT.begin(), entry.RTT.size() * 99 / 100))));
  RTTpercentiles.AddLItem(percentile);

  putReq.AddItem("RTTPS", RTTpercentiles);

  // RTT bucket item count
  Aws::DynamoDB::Model::AttributeValue RTTbucketCount;

  unsigned long long bucketWidth =
      (*(--entry.RTT.end()) - *entry.RTT.begin()) / 10;

  std::multiset<unsigned long long>::iterator bucketHead = entry.RTT.begin();
  std::multiset<unsigned long long>::iterator bucketTail;
  unsigned long long bucketStart = *bucketHead;

  std::shared_ptr<Aws::DynamoDB::Model::AttributeValue> count;

  for (unsigned char i = 0; i < 9; ++i) {
    bucketStart += bucketWidth;
    bucketTail = entry.RTT.upper_bound(bucketStart);

    count = std::make_shared<Aws::DynamoDB::Model::AttributeValue>();
    count->SetN(int(std::distance(bucketHead, bucketTail)));
    RTTbucketCount.AddLItem(count);

    bucketHead = bucketTail;
  }

  count = std::make_shared<Aws::DynamoDB::Model::AttributeValue>();
  count->SetN(int(std::distance(bucketHead, entry.RTT.end())));
  RTTbucketCount.AddLItem(count);

  putReq.AddItem("RTTBC", RTTbucketCount);

  // Average HDS
  Aws::DynamoDB::Model::AttributeValue averageHDS;
  averageHDS.SetN(std::to_string(entry.HDSSum / entry.HDSCount));
  putReq.AddItem("AvgHDS", averageHDS);

  // Average RES
  Aws::DynamoDB::Model::AttributeValue averageRES;
  averageRES.SetN(std::to_string(entry.RESSum / entry.RESCount));
  putReq.AddItem("AvgRES", averageRES);

  const Aws::DynamoDB::Model::PutItemOutcome &putOutcome =
      client.PutItem(putReq);

  if (!putOutcome.IsSuccess()) {
    std::cout << "Failed to put stat entry\n"
              << putOutcome.GetError().GetMessage() << std::endl;
    exit(PUTSTATENTRYERRNUM);
  }
}

#pragma once

// Test latency between all instances
void testLatency(
    Aws::DynamoDB::DynamoDBClient &,
    std::vector<std::pair<Aws::String, Aws::EC2::Model::Instance> &> &,
    const int &, const int &, const int &);

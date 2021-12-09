# EC2 network latency project
# by Jung Chak and Jamison Heiner

## TL;DR (how to use)

### Setup Gru

1.  Setup an EC2 instance _with at least 16GB of RAM_ (allowing AWS SDK be built from source), running Debian OS (for access to `apt` package manager). And a security group that provides SSH access.
2.  SSH into the instance.
3.  Run the following commands/do the following:
    1.  `yes | sudo apt update`
    2.  `yes | sudo apt upgrade`
    3.  `yes | sudo apt install git`
    4.  `yes | git clone https://github.com/chakjung/ec2LT.git`
        `# (this repo)`
    5.  `cd ec2LT`
    6.  `yes | sudo ./gruInit.sh`
        `# (this sets up chrony, builds the AWS SDK from source, and a few other things -- it will take a few minutes)`
    7.  `cd gru`
    8.  Add this instance's public and private IP addresses to `gru/config.h`
    9.  `cd build`
    10. `./build.sh`
    11. Add the appropriate AWS credentials to `~/.aws/credentials` ([click for AWS documentation](https://docs.aws.amazon.com/cli/latest/userguide/cli-configure-files.html))
    12. `./gru > output 2>&1 &`
        `# (this starts Gru; note that the clock synchronization process takes around 3 hours)`
        `# (this also redirects stdout and stderr to a file "output" and runs the process in the background)`

### Tear everything down prematurely

#### From somewhere that isn't an EC2 instance

1. Download and install Git
2. Clone this repository
3. Navigate to the `bigRedButton` directory, build it using the `build.sh` script under `build`, and run it using `./bigRedButton` -- **note:** if there are any security groups on your account that you want to preserve, add their names to the `SGWHITELIST` constant in `bigRedButton/config.h` before building.

#### From an EC2 instance

Same as above, but before building and executing, add the EC2's instance ID to the `INSTANCEWHITELIST` constant in `bigRedButton/config.h`

## System Architecture

There are three main components to the back end: Gru, Minions, and the Big Red Button. Each of these components has its own subdirectory in this repository. Each component can be configured in its `config.h` file, it can be built using its `build/build.sh` file (with the exception of the Minion -- use `make -f minion.makefile` instead), and can be run using `./<componentNameHere>`.

### Gru (AKA "Swarm Conductor")

This is the main component of the back end. It is the EC2 instance in charge of creating the "Minion" EC2 instances (one for each AZ) and for traffic control of latency tests among these Minions. To run this project, simply build and start Gru on an EC2 instance as described above.

As part of its setup, it synchronizes the system clock with [ATSS](https://aws.amazon.com/about-aws/whats-new/2017/11/introducing-the-amazon-time-sync-service/), and configures chrony to improve measurement precision. This process takes ~3 hours. Gru also automatically creates the DynamoDB tables, to store the latency data (if they don't already exist).

### Minion (AKA "Swarm Instance")

This is the component, responsible for running the latency tests. As controlled by Gru, a pair of Minions will establish a TCP connection, run 10 latency tests, and report the results back to Gru. These Minions are automatically stood up, configured, and terminated by Gru, using a startup script and the AWS SDK. This startup script is located in `gru/gru.cpp`.

### Big Red Button

This component is used in emergencies only, typically. It will remove any running EC2 instances, and any security groups associated with your account. It's possible to whitelist instances, and security groups, so that they are safe from this script. See above for usage instructions.

## Customization

There are many things to customize in each component's `config.h` file.  
An example is the number of latency tests, executed per pair of connection.

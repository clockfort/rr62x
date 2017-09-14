# DKMS Support

Using DKMS with rr62x driver (at least on Ubuntu)

## Installation

While cd'd to the root of this repo (where the dkms.conf file is)

sudo cp -R . /usr/src/rr62x-1.2
sudo dkms add -m rr62x -v 1.2
sudo dkms build -m rr62x -v 1.2
sudo dkms install -m rr62x -v 1.2
sudo modprobe rr62x

## References

Adapted from [Ubuntu Help](https://help.ubuntu.com/community/RocketRaid)


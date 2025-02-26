FROM debian:9

# Set up Debian archive sources
RUN echo "deb http://archive.debian.org/debian stretch contrib main non-free" > /etc/apt/sources.list && \
    echo "deb http://archive.debian.org/debian-security stretch/updates main" >> /etc/apt/sources.list && \
    apt-get update && \
    apt-get -y install --allow-change-held-packages python git curl jq python3 python3-pip libffi-dev && \
    pip3 install --upgrade cryptography==2.9.2 awscli && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Rename Wazuh-related tools to DefendX branding
ADD wpkpack.py /usr/local/bin/defendx-wpkpack
ADD run.sh /usr/local/bin/defendx-run

# Create required directories for DefendX Agent
VOLUME /var/local/defendx
VOLUME /etc/defendx
VOLUME /etc/defendx/checksum

# Set execute permissions for scripts
RUN chmod +x /usr/local/bin/defendx-wpkpack /usr/local/bin/defendx-run

# Set entrypoint
ENTRYPOINT ["/usr/local/bin/defendx-run"]

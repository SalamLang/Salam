#!/bin/sh
# Issues and renews the salamlang.ir + *.salamlang.ir certificate (DNS-01 via
# Cloudflare). Installed as /opt/certbot/renew-salamlang-wildcard.sh.
#
# Needs /opt/certbot/cloudflare/salamlang.ini (mode 600) with one line:
#   dns_cloudflare_api_token = <token limited to Zone:DNS:Edit on salamlang.ir>
#
# First issue (once the token file exists):
#   sh /opt/certbot/renew-salamlang-wildcard.sh issue
# Cron (daily, like the others):
#   37 4 * * * /opt/certbot/renew-salamlang-wildcard.sh >> /var/log/certbot-salamlang-wildcard.log 2>&1
set -eu

run() {
    docker run --rm -v /etc/letsencrypt:/etc/letsencrypt -v /opt/certbot/cloudflare:/cf certbot/dns-cloudflare "$@"
}

reload() {
    docker exec nginx nginx -t && docker exec nginx nginx -s reload
}

if [ "${1:-}" = issue ]; then
    run certonly --dns-cloudflare --dns-cloudflare-credentials /cf/salamlang.ini \
        --dns-cloudflare-propagation-seconds 30 --non-interactive --agree-tos \
        --account 3e78a1477e5685d945e509a52c7da32b --key-type ecdsa \
        --cert-name salamlang-wildcard -d salamlang.ir -d '*.salamlang.ir'
    reload
    exit 0
fi

run renew --cert-name salamlang-wildcard --quiet && reload

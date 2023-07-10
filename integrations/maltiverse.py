#!/var/ossec/framework/python/bin/python3

"""

What is Maltiverse?
###################

Maltiverse works as a broker for Threat intelligence sources that are
aggregated from more than a hundred different Public, Private and Community
sources. Once the data is ingested, the IoC Scoring Algorithm applies a
qualitative classification to the IoC that changes. Finally this data can
be queried in a Threat Intelligence feed that can be delivered to your
Firewalls, SOAR, SIEM, EDR or any other technology.


What does this integration?
###########################

This integration enrichs any alert generated by Wazuh via the Maltiverse API,
inserting new fields in case of match and following the threat taxonomy of the
ECS standard (Elastic Common Squema).

https://www.elastic.co/guide/en/ecs/current/ecs-threat.html

Ipv4, Domain names, Urls and MD5/SHA1 checksums are checked in Maltiverse
platform in order to enrich the original alert with threat Intel information

Installation Guide
##################

1. Move this file ``maltiverse.py`` to ``/var/ossec/integrations/custom-maltiverse.py``
   and make sure the file has the right perms:

   chmod +x /var/ossec/integrations/custom-maltiverse.py
   chown root.wazuh /var/ossec/integrations/custom-maltiverse.py

2. Add this to the ossec.conf file, inside <ossec_config></ossec_config> block:

<integration>
     <name>custom-maltiverse</name>
     <hook_url>https://api.maltiverse.com</hook_url>
     <api_key><YOUR_MALTIVERSE_AUTH_TOKEN></api_key>
     <alert_format>json</alert_format>
</integration>

3. Restart Wazuh Manager:

    /etc/init.d/wazuh-manager restart

"""

import json
import hashlib
import ipaddress
import os
from socket import socket, AF_UNIX, SOCK_DGRAM
import sys
import time

import requests

# Global vars
debug_enabled: bool = False
pwd: str = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
json_alert: dict = {}
now: str = time.strftime("%a %b %d %H:%M:%S %Z %Y")

# Set paths
LOG_FILE: str = f"{pwd}/logs/integrations.log"
SOCKET_ADDR: str = f"{pwd}/queue/sockets/queue"


class Maltiverse:
    """This class is a simplification of maltiverse pypi package."""

    def __init__(
        self, endpoint: str = "https://api.maltiverse.com", auth_token: str = None
    ):
        self.endpoint = endpoint
        self.auth_token = auth_token
        self.headers = {
            "Accept": "application/json",
            "Authorization": f"Bearer {self.auth_token}",
        }

    def ip_get(self, ip_addr: str) -> dict:
        return requests.get(
            f"{self.endpoint}/ip/{ip_addr}", headers=self.headers
        ).json()

    def hostname_get(self, hostname: str) -> dict:
        return requests.get(
            f"{self.endpoint}/hostname/{hostname}", headers=self.headers
        ).json()

    def url_get(self, url: str) -> dict:
        urlchecksum = hashlib.sha256(url.encode("utf-8")).hexdigest()
        return requests.get(
            f"{self.endpoint}/url/{urlchecksum}", headers=self.headers
        ).json()

    def sample_get(self, sample: str, algorithm: str = "md5") -> dict:
        """Requests a sample"""
        mapping = {
            "md5": self.sample_get_by_md5,
            "sha1": self.sample_get_by_sha1,
        }
        return mapping.get(algorithm, mapping.get("md5"))()

    def sample_get_by_md5(self, md5: str):
        """Requests a sample by MD5"""
        return requests.get(
            f"{self.endpoint}/sample/md5/{md5}", headers=self.headers
        ).json()

    def sample_get_by_sha1(self, sha1: str):
        """Requests a sample by SHA1"""
        return requests.get(
            f"{self.endpoint}/sample/sha1/{sha1}", headers=self.headers
        ).json()


def main(args: list):
    global debug_enabled
    try:
        # Read arguments
        bad_arguments = False
        if len(args) >= 4:
            msg = "{0} {1} {2} {3} {4}".format(
                now,
                args[1],
                args[2],
                args[3],
                args[4] if len(args) > 4 else "",
            )
            debug_enabled = len(args) > 4 and args[4] == "debug"
        else:
            msg = f"{now} Wrong arguments"
            bad_arguments = True

        # Logging the call
        with open(LOG_FILE, "a") as f:
            f.write(msg + "\n")

        if bad_arguments:
            debug(f"# Exiting: Bad arguments. Inputted: {args}")
            sys.exit(2)

        # Main function
        process_args(args)

    except Exception as e:
        debug(str(e))
        raise


def process_args(args: list):
    debug("# Starting")

    alert_file_location = args[1]
    api_key: str = args[2]
    hook_url: str = args[3]

    debug(f"# File location: {alert_file_location}")
    debug(f"# API Key: {api_key}")
    debug(f"# Hook Url: {hook_url}")

    # Load alert. Parse JSON object.
    try:
        with open(alert_file_location) as alert_file:
            json_alert = json.load(alert_file)
    except FileNotFoundError:
        debug("# Alert file %s doesn't exist" % alert_file_location)
        sys.exit(3)
    except json.decoder.JSONDecodeError as e:
        debug(f"Failed getting json_alert: {e}")
        sys.exit(4)

    debug(f"# Processing alert: {json_alert}")

    maltiverse_api = Maltiverse(endpoint=hook_url, auth_token=api_key)

    # Request Maltiverse info and send event to
    # Wazuh Manager in case of positive match
    for msg in request_maltiverse_info(json_alert, maltiverse_api):
        send_event(msg, json_alert["agent"])


def debug(msg: str):
    if debug_enabled:
        msg = "{0}: {1}\n".format(now, msg)
        print(msg)
        with open(LOG_FILE, "a") as f:
            f.write(msg)


def get_ioc_confidence(ioc: dict) -> str:
    """Get vendor-neutral confidence rating

    using the None/Low/Medium/High scale defined in Appendix A
    of the STIX 2.1 framework
    """
    if not (classification := ioc.get("classification")):
        return "Not Specified"

    sightings = len(ioc.get("blacklist", []))
    if classification == "malicious":
        return "High" if sightings > 1 else "Medium"
    elif classification == "suspicious":
        return "Medium" if sightings > 1 else "Low"
    elif classification in ("neutral", "whitelist"):
        return "Low" if sightings > 1 else "None"


def maltiverse_alert(
    alert_id: int,
    ioc_dict: dict,
    ioc_name: str,
    ioc_ref: str,
    include_full_source: bool = True,
) -> dict:
    """Generate a new alert using Elastic Common Schema (ECS) Threat Fields

    If ``include_full_source`` is True, the complete
    Maltiverse API response is also included.
    """

    _blacklist = ioc_dict.get("blacklist", [])
    _type = ioc_dict.get("type")

    alert = {
        "integration": "maltiverse",
        "alert_id": alert_id,
        "maltiverse": {
            "source": ioc_dict,
        },
        "threat": {
            "indicator": {
                "name": ioc_name,
                "type": _type,
                "description": ", ".join(
                    set([b.get("description") for b in _blacklist]),
                ),
                "provider": ", ".join(
                    set([b.get("source") for b in _blacklist]),
                ),
                "first_seen": ioc_dict.get("creation_time"),
                "modified_at": ioc_dict.get("modification_time"),
                "last_seen": ioc_dict.get("modification_time"),
                "confidence": get_ioc_confidence(ioc_dict),
                "sightings": len(_blacklist),
                "reference": f"https://maltiverse.com/{_type}/{ioc_ref}",
            }
        },
    }

    if not include_full_source:
        alert.pop("maltiverse")

    return alert


def request_maltiverse_info(alert: dict, maltiverse_api: Maltiverse) -> dict:
    results = []

    if "syscheck" in alert and "md5_after" in alert["syscheck"]:
        debug("# Maltiverse: MD5 checksum present in the alert")
        md5 = alert["data"]["md5_after"]

        if md5_ioc := maltiverse_api.sample_get_by_md5(md5):
            results.append(
                maltiverse_alert(
                    alert_id=alert["id"],
                    ioc_dict=md5_ioc,
                    ioc_name=md5,
                    ioc_ref=md5,
                )
            )

    if "syscheck" in alert and "sha1_after" in alert["syscheck"]:
        debug("# Maltiverse: SHA1 checksum present in the alert")
        sha1 = alert["data"]["sha1_after"]

        if sha1_ioc := maltiverse_api.sample_get_by_sha1(sha1):
            results.append(
                maltiverse_alert(
                    alert_id=alert["id"],
                    ioc_dict=sha1_ioc,
                    ioc_name=sha1,
                    ioc_ref=sha1,
                )
            )

    if "data" in alert and "srcip" in alert["data"]:
        debug("# Maltiverse: Source IP Address present in the alert")
        ipv4 = alert["data"]["srcip"]

        if not ipaddress.IPv4Address(ipv4).is_private:
            if ipv4_ioc := maltiverse_api.ip_get(ipv4):
                results.append(
                    maltiverse_alert(
                        alert_id=alert["id"],
                        ioc_dict=ipv4_ioc,
                        ioc_name=ipv4,
                        ioc_ref=ipv4,
                    )
                )

    if "data" in alert and "hostname" in alert["data"]:
        debug("# Maltiverse: Hostname present in the alert")
        hostname = alert["data"]["hostname"]

        if hostname_ioc := maltiverse_api.hostname_get(hostname):
            results.append(
                maltiverse_alert(
                    alert_id=alert["id"],
                    ioc_dict=hostname_ioc,
                    ioc_name=hostname,
                    ioc_ref=hostname,
                )
            )

    if "data" in alert and "url" in alert["data"]:
        debug("# Maltiverse: Url present in the alert")
        url = alert["data"]["url"]
        urlchecksum = hashlib.sha256(url.encode("utf-8")).hexdigest()

        if url_ioc := maltiverse_api.url_get(urlchecksum):
            results.append(
                maltiverse_alert(
                    alert_id=alert["id"],
                    ioc_dict=url_ioc,
                    ioc_name=url,
                    ioc_ref=urlchecksum,
                )
            )

    return results


def send_event(msg: str, agent: dict = None):
    if not agent or agent["id"] == "000":
        string = f"1:maltiverse:{json.dumps(msg)}"
    else:
        location = "[{0}] ({1}) {2}".format(
            agent["id"],
            agent["name"],
            agent["ip"] if "ip" in agent else "any",
        )
        location = location.replace("|", "||").replace(":", "|:")
        string = f"1:{location}->maltiverse:{json.dumps(msg)}"

    debug(string)
    sock = socket(AF_UNIX, SOCK_DGRAM)
    sock.connect(SOCKET_ADDR)
    sock.send(string.encode())
    sock.close()


if __name__ == "__main__":
    main(sys.argv)

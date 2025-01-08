import os
import shlex
import requests
from bs4 import BeautifulSoup

repo = "SalamLang/Salam"

def create_github_issue(repo, title, body, labels=None):
    escaped_body = shlex.quote(body).replace("'", '"').replace("\n", "<br>")
    
    if labels:
        labels_option = ' '.join([f'--label "{label}"' for label in labels])
    else:
        labels_option = ""

    command = f'gh issue create --repo {repo} --title "{title}" --body {escaped_body} {labels_option}'
    
    print(command)
    os.system(command)

url = "https://www.w3schools.com/tags/default.asp"
response = requests.get(url)

if response.status_code == 200:
    soup = BeautifulSoup(response.content, 'html.parser')

    table = soup.find('table', {'class': 'ws-table-all notranslate'})

    result = []

    rows = table.find_all('tr')[1:]
    for row in rows:
        cols = row.find_all('td')
        if len(cols) == 2:
            tag_element = cols[0].find('a')
            name = tag_element.text if tag_element else cols[0].text.strip()
            link = ("https://www.w3schools.com/tags/" + tag_element['href']) if tag_element else ""

            brief = cols[1].text.strip()

            result.append({
                "name": name,
                "link": link,
                "brief": brief
            })

    for item in result:
        title = f"Implement support for `{item['name']}` tag (HTML)"
        body = (
            f"### Description\n\n"
            f"Tag: `{item['name']}`\n\n"
            f"**Brief:** {item['brief']}\n\n"
            f"[Learn more about `{item['name']}`]({item['link']})\n\n"
            f"Implement this tag in the Salam programming language for generating equivalent HTML."
        )

        labels = ["html", "enhancement", "good first issue", "yaml"]
        
        create_github_issue(repo, title, body, labels)
        # break
        print(f"Issue created for {item['name']}")
else:
    print(f"Failed to fetch the page. Status code: {response.status_code}")

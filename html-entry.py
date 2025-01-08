import requests
from bs4 import BeautifulSoup

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

    print(result)
else:
    print(f"Failed to fetch the page. Status code: {response.status_code}")

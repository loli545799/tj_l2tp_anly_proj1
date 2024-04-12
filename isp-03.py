import requests
from bs4 import BeautifulSoup
import os

# 目标网页的URL
url = 'https://www.cvedetails.com/vulnerability-list/vendor_id-217/Openbsd/Openbsd.html'

headers = {'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/123.0.0.0 Safari/537.36 Edg/123.0.0.0'}

# 发送GET请求，获取网页内容
response = requests.get(url,headers=headers)
web_content = response.text
# 用于存储txt文件链接的列表
txt_links = []
t_links = []

if response.status_code == 200:
    # 解析HTML内容
    soup = BeautifulSoup(web_content, 'html.parser')

    # 查找所有<a>标签
    links = soup.find_all('a', href=True)

    # 筛选出以/cve/开头的链接
    for link in links:
        href = link['href']
        if href.startswith('/cve/CVE-'):
            txt_links.append(href)

    # 下载并保存每个txt文件
    for txt_link in txt_links:
        # 这里需要根据实际情况构造完整的文件下载URL
        file_url = 'https://www.cvedetails.com' + txt_link
        response = requests.get(file_url,headers=headers)
        if response.status_code == 200:
            soup = BeautifulSoup(response.text,'html.parser')

            Sec_links = soup.find_all('a',href=True)

            for Sec_link in Sec_links:
                Sec_href = Sec_link['href']
                if Sec_href.startswith('/version-search.php?cpeMatchCriteriaId='):
                    t_links.append(Sec_href)

            for t_link in t_links:
                final_url = 'https://www.cvedetails.com' + t_link
                response = requests.get(final_url,headers=headers)
                if response.status_code == 200:

                    soup = BeautifulSoup(response.text,'html.parser')

                    tbody = soup.find('tbody')

                    if tbody:
                        for tr in tbody.find_all('tr'):
                            fourth_td = tr.find_all('td')[3] if len(tr.find_all('td')) >= 4 else None
                            if fourth_td:
                                text = fourth_td.get_text(strip=True)
                                with open('versions.txt', 'a', encoding='utf-8') as file:
                                    file.write(text + '\n')

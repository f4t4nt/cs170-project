from selenium import webdriver
from webdriver_manager.chrome import ChromeDriverManager
import time

driver = webdriver.Chrome(ChromeDriverManager().install())
driver.get("https://170-leaderboard.vercel.app/team/gamers")

table = dict()
time.sleep(5)

for row in driver.find_elements('xpath', '//tr'):
    cells = row.find_elements('xpath', './/td')
    if len(cells) == 2:
        table[cells[0].text] = cells[1].text

print(table)
from selenium import webdriver
from webdriver_manager.chrome import ChromeDriverManager

driver = webdriver.Chrome(ChromeDriverManager().install())
driver.get("https://170-leaderboard.vercel.app/team/discordggTYsy64VcWT")

data = driver.execute_script("return document.body.innerText")
table = dict()
for line in data.splitlines():
    if line.startswith("small") or line.startswith("medium") or line.startswith("large"):
        line = line.split()
        table[line[0] + line[1]] = (float(line[2]), int(line[3]))
sorted_table = sorted(table.items(), key=lambda item: item[1][1], reverse=True)
for i in sorted_table:
    if i[1][1] != 1:
        print(i)
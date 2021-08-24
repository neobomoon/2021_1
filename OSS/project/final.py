import discord 
import asyncio
from discord.ext import commands
from bs4 import BeautifulSoup
import urllib.request as req

client = discord.Client()



def Movie_title_crawling():
    code = req.urlopen("http://www.cgv.co.kr/common/showtimes/iframeTheater.aspx?areacode=01&theatercode=0097&date=20210607") #포항
    soup = BeautifulSoup(code, "html.parser")
    movies = soup.select('body > div > div.sect-showtimes > ul > li')

    output_result = []
    result = ""
    for movie in movies:
        title = movie.select_one('div > div.info-movie > a > strong').get_text().strip()
        # title = title.replace("-", " ")
        # print(title)
        output_result.append(title)
        result += title + "\n"
    return output_result, result


def Movie_timetable_crawling(movie_want): #해당 영화 시간표 출력 + 자리
    code = req.urlopen("http://www.cgv.co.kr/common/showtimes/iframeTheater.aspx?areacode=01&theatercode=0097&date=20210607") #포항
    soup = BeautifulSoup(code, "html.parser")
    movies = soup.select('body > div > div.sect-showtimes > ul > li')

    movie_to_search = ""
    for movie in movies:
        title = movie.select_one('div > div.info-movie > a > strong').get_text().strip()
        # title = title.replace("-", " ")
        if(movie_want == title):
            movie_to_search = movie

    
    time_info =[]
    timetables = movie_to_search.select('div > div.type-hall > div.info-timetable > ul > li')
    print(timetables)
    for timetable in timetables:
        print(f"\n{timetable}\n")
        if timetable.select_one('span').get_text().strip() == "마감":
            time = timetable.select_one('em').get_text()
            time_seat = (time, "마감(현장 예약 가능)")
            time_info.append(time_seat)
            continue
        time = timetable.select_one('a > em').get_text()
        seat = timetable.select_one('a > span').get_text()
        time_seat = (time,seat)
        time_info.append(time_seat)
    return time_info

token = "ODUxMjgyMjMzMTI1MjQwODUy.YL2AXA.lC8THptc5kYlVAvqM7X__9-JsZI"


@client.event
async def on_ready():

    print(client.user.name)
    print('Logged in, bot is ready to run')
    game = discord.Game("서비스")
    await client.change_presence(status = discord.Status.online, activity = game)


@client.event
async def on_message(message):
    movies, movie = Movie_title_crawling()

    if message.content == "영화":
        await message.channel.send(f"today movies\n------------\n{movie}------------")

    if movies.count(message.content) != 0:
        timetable = Movie_timetable_crawling(message.content)
        await message.channel.send(f"{message.content} : {timetable}")


client.run(token)

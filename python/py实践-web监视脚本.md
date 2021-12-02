# python实践：Web监视脚本

## 背景

这不，刚面试完，等结果等的好焦虑啊，总想时不时打开网站看看有没有新通知，但总是手动去做这个事情感觉很麻烦，于是就想到了用python做这件事很方便，于是捡起多年未看的python来探索一番



目标：

1. 每十五分钟请求一下目标网站，获取其HTML
2. 将HTML进行解析，搜索今天的日期字符串“2021-06-18”
3. 如果搜索到了，就将该日期所对应的通知标题和链接获取下来，通过某种方式发送到我的手机上

## 实现

```python
from requests import get
from bs4 import BeautifulSoup
from time import sleep,localtime,strftime
from random import randint
from smtplib import SMTP,SMTPException
from email.mime.text import MIMEText
from email.header import Header

# 全局变量
# 目标站点URL
szTargetURL = "https://cs.xidian.edu.cn/tzgg.htm"
szTargetURLRoot = "https://cs.xidian.edu.cn/"
szTodayTime = ""
szNewMessage = []


def GetTargetHTML():
    # 请求头
    headers = {'user-agent':'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.106 Safari/537.36'}
    res = get(szTargetURL,headers=headers)
    res.encoding = 'utf-8'
    return res.text


def ParseHTML(szHTML:str):
    global szNewMessage
    soup = BeautifulSoup(szHTML,'lxml')
    for i in range(15):
        soupTag = soup.select(f"#line_u15_{i}")
        szLink = szTargetURLRoot + soupTag[0].a['href']
        szTitle = soupTag[0].a.string
        szTime = soupTag[0].span.string
        if szTime == szTodayTime:
            szRet = szTitle + "：" + szLink
            if szRet in szNewMessage:
                continue

            szNewMessage.append(szRet)
            return True
    return False


def SendEmail(szStr:str):
    # 第三方 SMTP 服务
    mail_host="smtp.qq.com"         # 设置服务器
    mail_user="3160318909@qq.com"   # 用户名
    mail_pass="vkswhmqswlehdejd"    # 口令 
    
    sender = "3160318909@qq.com"
    receivers = ['529250247@qq.com']

    message = MIMEText(szStr,'plain','utf-8')
    message['From'] = Header("selphBot",'utf-8')
    message['To'] = Header("大哥",'utf-8') 
    message['Subject'] = "【新通知】西电官网有新消息了！"

    try:
        smtpObj = SMTP()
        smtpObj.connect(mail_host,587)
        smtpObj.login(mail_user,mail_pass)
        smtpObj.sendmail(sender,receivers,message.as_string())
        print("Success")
    except:
        print(f"Error：{SMTPException}")


def main():
    global szTodayTime
    szTodayTime = strftime("%Y-%m-%d",localtime())
    # 测试用
    szTodayTime = "2021-06-16"
    print(f"CurrentDate:{szTodayTime}")
    while True:
        # 确保使用的日期是最新的
        #if szTodayTime != strftime("%Y-%m-%d",localtime()):
        #    szTodayTime = strftime("%Y-%m-%d",localtime())
        # 获取目标URL的HTML
        szHTML = GetTargetHTML()
        # 解析HTML，查询新信息
        bRet = ParseHTML(szHTML)
        if bRet:
            print(szNewMessage[-1])
            SendEmail(szNewMessage[-1])
        # 等会再看看有没有新消息
        sleep(randint(30*60,60*60))


# 当前模块__name__的值是__main__，在导入的模块里则值为模块名
# 用于确保脚本是从主文件中开始执行的
if __name__ == "__main__": 
    main()
```


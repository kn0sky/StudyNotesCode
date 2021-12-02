# 可复用代码片段

## 使用QQ邮箱SMTP发送邮件

```python
from smtplib import SMTP,SMTPException
from email.mime.text import MIMEText
from email.header import Header

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
    except(SMTPException) as e:
        print(f"Error：{e}")
```

## GET请求获取HTML

```python
from requests import get
from bs4 import BeautifulSoup

def GetTargetHTML():
    # 请求头
    headers = {'user-agent':'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.106 Safari/537.36'}
    res = get(szTargetURL,headers=headers)
    res.encoding = 'utf-8'
    return res.text
```


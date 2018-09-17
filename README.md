### SQLAdvisor-HTTP 简介
SQLAdvisor是由美团点评公司技术工程部DBA团队（北京）开发维护的一个分析SQL给出索引优化建议的工具。SQLAdvisor-HTTP扩展了SQLAdvisor的HTTP服务能力，可以更方便的与其他内部系统联动

原地址如下：https://github.com/Meituan-Dianping/SQLAdvisor

### Docker化HTTP服务安装

#### 基于美团SQLAdvisor新增HTTP服务支持

git clone https://github.com/evenzhang/SQLAdvisor-HTTP.git

cd SQLAdvisor-HTTP/docker/

docker build -t sqladvisor .

docker run -d -p 8081:8081 -v /usr/local/sqladvisor/etc:/SQLAdvisor-HTTP/sqladvisor-http/etc -i -t sqladvisor

#### 测试
curl -d "host=192.168.1.100&port=3306&user=root&password=root&db=test&sql=select project_name,count(*) as cnt from repos,changelog where repos.repos_id = changelog.repos_id and changelog.create_time < '2018-01-01 00:00:00' group by project_name limit 10" "http://192.168.1.101:8081/check/service"


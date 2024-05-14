#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

// 全局的watcher观察器   zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type,
                   int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  // 回调的消息类型是和会话相关的消息类型（连接或者断开连接，会话超时）
	{
		if (state == ZOO_CONNECTED_STATE)  // zkclient和zkserver连接成功
		{
			sem_t *sem = (sem_t*)zoo_get_context(zh);//从指定的句柄上获取信号量
            sem_post(sem);//信号量加1
		}
	}
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)//句柄不为空，代表已经和zkserver连接成功
    {
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源  MySQL_Conn
    }
}

// 连接zkserver
void ZkClient::Start()
{
	//MprpcApplication::GetInstance() 获得MprpcApplication类创建的对象
	//MprpcApplication::GetInstance().GetConfig()获得MprpcConfig类型的静态对象
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    
	/*
	zookeeper_mt：多线程版本
	zookeeper的API客户端程序提供了三个线程
	1、API调用线程（调用start会执行zookeeper_init ）
	2、网络I/O线程  调用pthread_create创建一个线程进行网络I/O连接  poll实现
	3、watcher回调线程 pthread_create 给客户端通知消息（zkserver给zkclient的通知）
	global_watcher 回调函数 
	30000：超时时间 30s
	m_zhandle：返回句柄
	*/
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle) //m_zhandle不为NULL表示创建句柄成功，而不是与zkserver连接成功
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);//给句柄绑定了信号量

    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
}

//path 节点的路径，data表示要创建的节点存放的数据，datalen表示data字节长度，state表示是否永久性的还是临时性的
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(m_zhandle, path, 0, nullptr);
	if (ZNONODE == flag) // 表示path的znode节点不存在
	{
		// 创建指定path的znode节点了 ZOO_OPEN_ACL_UNSAFE与权限相关 
		//state 默认是永久性节点，如果有ZOO_EPHEMERAL则表示是临时节点 ephemeralOwner=0X0表示是永久性节点，否则是临时性节点
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (flag == ZOK)
		{
			std::cout << "znode create success... path:" << path << std::endl;
		}
		else
		{
			std::cout << "flag:" << flag << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

// 根据指定的path，获取znode节点的值（rpc服务的ip和端口号）
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK)
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
	else
	{
		return buffer;
	}
}
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread.h"
#include <iostream>
using namespace std;

base::Lock g_lock;
class Foo 
    : public base::RefCountedThreadSafe<Foo> {
public:
    Foo(int t) 
      : a_(t),
        worker_pool_(new base::SequencedWorkerPool(3, "fangr_")) { 
    }

    ~Foo() {
        worker_pool_->Shutdown();
    }

    void work() {
        worker_pool_->PostWorkerTask(FROM_HERE, base::Bind(&Foo::DoWork, this));
    }

    void ordered_tasks() {
        base::SequencedWorkerPool::SequenceToken token = worker_pool_->GetSequenceToken();

        worker_pool_->PostSequencedWorkerTask(token, FROM_HERE, base::Bind(&Foo::DoTask1, this));

        worker_pool_->PostSequencedWorkerTask(token, FROM_HERE, base::Bind(&Foo::DoTask2, this));
    }

private:
    void DoWork() {  
        for (int i = 0; i < a_; ++i) {
            base::AutoLock lock(g_lock);
            cout << i << ",DoWork" << endl;
        }
    }

    void DoTask1() {
        for (int i = 0; i < a_; ++i) {
            base::AutoLock lock(g_lock);
            cout << i << ",DoWork1" << endl;
        }
    }

    void DoTask2() {
        for (int i = 0; i < a_; ++i) {
            base::AutoLock lock(g_lock);
            cout << i << ", DoWork2" << endl;
        }
    }

private:
    scoped_refptr<base::SequencedWorkerPool> worker_pool_;

    int a_;
};

void test_Foo() {
    scoped_refptr<Foo> test(new Foo(10));

    int count = 10;
    while (count > 0) {
        --count;

        test->work();
        test->ordered_tasks();
    }
}

class MyObj : public base::RefCountedThreadSafe<MyObj> {
public:
    MyObj() {
        std::cout << "construct" << std::endl;
    }

    ~MyObj() {
        std::cout << "destroy" << std::endl;
    }
};
void test_scoped_refptr() {
    scoped_refptr<MyObj> my_obj(new MyObj());
}

void test_scoped_ptr() {
    scoped_ptr<std::string> my_obj(new std::string());
}

#include "base/threading/simple_thread.h"

class MyDelegateObj : public base::DelegateSimpleThread::Delegate {
public:
    MyDelegateObj() {
        std::cout << "construct" << std::endl;
    }

    ~MyDelegateObj() {
        std::cout << "destroy" << std::endl;
    }

    virtual void Run() {
        int cnt = 1000;
        while (cnt > 0) {
            --cnt;
            std::cout << "run:" << cnt << std::endl;
        }
    }
};
void test_DelegateSimpleThread() {
    MyDelegateObj my_delegate_obj;
    base::DelegateSimpleThread my_thread(&my_delegate_obj, "thread1");

    my_thread.Start();
    my_thread.Join();
}

void test_DelegateSimpleThreadPool() {
    MyDelegateObj my_delegate_obj;
    base::DelegateSimpleThreadPool my_thread_pool("thread2", 3);
    my_thread_pool.AddWork(&my_delegate_obj, 3);
    my_thread_pool.Start();
    my_thread_pool.JoinAll();
}



int main(int argc, char* argv[]) {
    return 0;
}

#pragma once

#include <queue>
#include <functional>
#include <vector>
#include <thread>
#include "BlockingQueue.h"

using namespace std;

namespace threadPool {
	class ThreadPool {
	private:
		BlockingQueue<function<void(void)>> taskList;


	public:
		ThreadPool(int theads);
		void execute(function<void(void)> fn);
		size_t taskCount();
	};


	ThreadPool::ThreadPool(int size) {
		for (int i = 0; i < size; i++) {
			thread* threadItem = new thread([this]() {
				while (true) {
					auto task = taskList.Take();
					task();
				}
			});
		}
	}

	void ThreadPool::execute(function<void(void)> task) {
		taskList.Put(task);
	}

	size_t ThreadPool::taskCount() {
		return taskList.Size();
	}
}


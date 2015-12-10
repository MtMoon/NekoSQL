/*
 * BPlusTree.h
 *
 *  Created on: 2015-12-6
 *      Author: yxy
 */

#ifndef BPLUSTREE_H_
#define BPLUSTREE_H_

#include <vector>
#include <string>
#include <map>

using namespace std;
//定义节点类
template <typename T> class BTNode {
public:
	BTNode* parent;
	vector<T> key;
	vector<BTNode<T>*> child;
	int pagetype; //0 非叶级页，1 叶级页
	BTNode() {parent = NULL; child.clear(); }
	BTNode(T e, BTNode<T>* lc = NULL, BTNode<T>* rc = NULL) {
		parent = NULL;
		key.push_back(e);
		child.push_back(lc);
		child.push_back(rc);
		if (lc != NULL) {
			lc->parent = this;
		}

		if (rc != NULL) {
			rc->parent = this;
		}
	}
};


//B+树
template <typename T> class BPlusTree {
public:
	BPlusTree(int ord);
	~BPlusTree();
	int const getOrder();
	int const getSize();
	BTNode<T>* getRoot();
	bool isEmpty();
	BTNode<T>* search(const T& e);
	bool insert(const T& e);
	bool remove(const T& e);

private:
	int size; //树内当前存放的关键码总数
	int order; //阶数
	BTNode<T>* root; //根节点
	BTNode<T>* hot; //用于搜索
	void solveOverflow(BTNode<T>* v); //处理上溢页分裂
	void solveUnderflow(BTNode<T>* v); //处理下溢页合并
	void release(BTNode<T>* v); //释放所有节点

	//自定义vector操作函数
	int vecSearch(vector<T> &vec, const T& e); //查找大于e的最小索引码值，获取其rank
	void vecInsert(vector<T> &vec, T e, int r); //在vector的特定位置插入值
	void vecRemove(vector<T> &vec, int r); //删除vec特定位置的值

};

#endif /* BPLUSTREE_H_ */

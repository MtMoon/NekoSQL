/*
 * BPlushTree.cpp
 *
 *  Created on: 2015-12-6
 *      Author: yxy
 */

#include "BPlusTree.h"

template<typename T>

BPlusTree<T>::BPlusTree (int ord = 3) {
	hot = NULL;
	order = ord;
	size = 0;
	root = new BTNode<T>();
}

template<typename T>
BPlusTree<T>::~BPlusTree() {
	release(root);
}

template<typename T>
int const BPlusTree<T>::getOrder() {
	return order;
}

template<typename T>
int const BPlusTree<T>::getSize() {
	return size;
}

template<typename T>
BTNode<T>* BPlusTree<T>::getRoot() {
	return root;
}

template<typename T>
bool BPlusTree<T>::isEmpty() {
	return (root == NULL);
}

/**
 * 查找节点并返回，对B+树来说，一定只能在叶节点命中
 */
template<typename T>
BTNode<T>* BPlusTree<T>::search(const T& e) {
	BTNode<T>* v = root;
	hot = NULL;
	while (v) {
		int rank = vecSearch(v->key, e);
		if (v->type == 0) {
			return v;
		}

		hot = v;
		v = v->child[rank+1];
	}
	return NULL; //查找失败
}


//插入索引码值
template<typename T>
bool BPlusTree<T>::insert(const T& e) {
	BTNode<T>* v = search(e);
	if (v) {
		return false;
	}
	int r = vecSearch(hot->key, e);
	vecInsert(hot->key, e, r+1);
	vecInsert(hot->child, NULL, r+2);
	size++;
	solveOverflow(hot);
	return true;
}


template<typename T>
bool BPlusTree<T>::remove(const T& e) {

}

template<typename T>
void BPlusTree<T>::release(BTNode<T>* v) {

}


//处理上溢页分裂
template<typename T>
void BPlusTree<T>::solveOverflow(BTNode<T>* v) {
	if ( order >= v->child.size() ) return; //递归基：当前节点并未上溢
	int  s = order / 2; //轴点（此时应有_order = key.size() = child.size() - 1）
	BTNode<T>* u = new BTNode<T>(); //注意：新节点已有一个空孩子
	for ( int j = 0; j < order - s - 1; j++ ) { //v右侧_order-s-1个孩子及关键码分裂为右侧节点u
		u->child.insert ( j, v->child.remove ( s + 1 ) ); //逐个移动效率低
		u->key.insert ( j, v->key.remove ( s + 1 ) ); //此策略可改进
	}

	u->child[order - s - 1] = v->child.remove ( s + 1 ); //移动v最靠右的孩子
	if ( u->child[0] ) //若u的孩子们非空，则
		for ( int j = 0; j < order - s; j++ ) {//令它们的父节点统一
			u->child[j]->parent = u; //指向u
		}
		BTNode<T>* p = v->parent; //v当前的父节点p
		if ( !p ) {
			root = p = new BTNode<T>();
			p->child[0] = v;
			v->parent = p;
		} //若p空则创建之
		int r = 1 + p->key.search ( v->key[0] ); //p中指向u的指针的秩
		p->key.insert ( r, v->key.remove ( s ) ); //轴点关键码上升
		p->child.insert ( r + 1, u );  u->parent = p; //新节点u与父节点p互联
		solveOverflow ( p ); //上升一层，如有必要则继续分裂——至多递归O(logn)层
}


//void solveUnderflow(BTNode<T>* v); //处理下溢页合并


//vector操作函数
// 返回大于e的最小索引码值所在的下标
template<typename T>
int BPlusTree<T>::vecSearch(vector<T>& vec, const T& e) {
	if (e<vec[0]) {
		return 0;
	} else if (e>=vec[vec.size()-1]) {
		return vec.size()-1;
	} else {
		for (int i=0; i<vec.size(); i++) { //这里默认vec中是按升序排序的的
			if (vec[i]>e) {
				return i;
			}
		}
	}
}

/*
//在vector的特定位置插入值
template<typename T>
void BPlusTree<T>::vecInsert(vector<T>& vec, T e, int r) {
	vector<T>::iterator it = vec.begin();
	vec.insert(it+r, e);
}*/

//删除vec特定位置的值
template<typename T>
void BPlusTree<T>::vecRemove(vector<T> &vec, int r) {
	vector<char>::iterator it = vec.begin();
	vec.erase(it+r);
}

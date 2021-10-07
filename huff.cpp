/*
	Reda Kaafarani
	10/05/2021
	kaafaranireda@gmail.com
*/

/** \file     huff.cpp
 *  \brief    Generates huffman code for a certain input, saves huffman tree, and can decode back the binary message if the save file is conserved
 */

#include "bits/stdc++.h"

using namespace std;

typedef struct node
{
	double proba;
	char c; 	//should be used only in leaf nodes
	node *left;
	node *right;
	
	node() : proba(0.0), c('$'),left(nullptr), right(nullptr) {} //default node constructor

	node(double p, char cc)
	{
		this->left = nullptr;
		this->right = nullptr;
		this->proba = p;
		this->c = cc;
	}
}node;

enum mode
{
	ENC=1,
	DEC=2
};

//This structure is used as argument for priority queue creation, it simply allows for sorting the least probable elements as highest priority (Min Heap)
struct node_comp
{
	bool operator() (const node* a, const node* b) const
	{
		return a->proba > b->proba;
	}
};


void calc_probas(string msg, std::map<char, double> &charWeights)
{
	int msgLen = (int) msg.length();
	for (int i = 0; i < msgLen; i++)
	{
		charWeights[msg[i]]++;
	}
	for (auto &elem : charWeights)
		elem.second/=(1.0*msgLen);
}

void tree_create(std::priority_queue<node*, vector<node*>, node_comp> &pq, std::map<char, double> &charWeights)
{
	node *nd; //Used to create nodes from the node structure, this is used to push leaf nodes in the priority queue
		
	//Create leaf nodes from charWeights map and push them into priority queue
	for (const auto &elem : charWeights)
	{
		nd = new node(elem.second, elem.first);
		pq.push(nd);
		
	}
 	while (pq.size() > 1)
	{
		//Pop the two least probable elements and create internal node, assign these elements as children leaf nodes
		node *one, *two, *newNode;
		one = pq.top();
		pq.pop();
		two = pq.top();
		pq.pop();
		newNode = new node();
		newNode->proba = one->proba + two->proba;
		newNode->right = one->proba > two->proba ? one : two;
		newNode->left = newNode->right==one? two : one;
		pq.push(newNode);
	}
}

void traverse_tree(node* nd, string s,std::map<char, string> &encodedChars)
{
	if (nd==nullptr)
		return;
	if (nd->c!='$')
		encodedChars.insert(std::pair<char, string>(nd->c, s));
	traverse_tree(nd->left, s+"0", encodedChars);
	traverse_tree(nd->right, s+"1", encodedChars);
}

string encode_msg(string msg, std::map<char, string> &encodedChars)
{
	char ch;
	string st;
	for (int i = 0; i < (int) msg.length();i++)
	{
		ch = msg[i];
		st += encodedChars.find(ch)->second;
	}
	return st;
}

void free_resources(node *nd)
{
	if (nd->left!=nullptr)
		free_resources(nd->left);
	if (nd->right!=nullptr)
		free_resources(nd->right);
	if (nd!=nullptr)
	{
		delete nd;
		nd = nullptr;
	}
}

void save_tree(node *nd, ofstream &outFile)
{
	if (nd != nullptr)
		outFile<<nd->proba<<" "<<nd->c<<'\n';
	if (nd->left!=nullptr)
		save_tree(nd->left, outFile);
	if (nd->right!=nullptr)
		save_tree(nd->right, outFile);
}

void load_tree(node *&nd, ifstream &inFile, std::priority_queue<node*, vector<node*>, node_comp> &pq)
{
	string s;
	if (getline(inFile, s))
	{
		double proba = stod(s.substr(0,s.find(' ')));
		char c = s[s.length()-1];
		if (c=='$')
		{
			nd = new node(proba, c);
			if (pq.empty())
				pq.push(nd); //push head
			load_tree(nd->left, inFile, pq);
			load_tree(nd->right, inFile, pq);
			
		}
		else
		{
			nd = new node(proba, c);
			return;
		}
	}
}

void decode_huff(node *nd, string msg)
{
	node *tnd = nd;
	for (int i = 0; i < (int) msg.size(); i++)
	{
		if (msg[i]=='0')
			tnd = tnd->left;
		if (msg[i]=='1')
			tnd = tnd->right;
		if (tnd!=nullptr && tnd->left == nullptr && tnd->right == nullptr)
		{
			cout<<tnd->c;
			tnd = nd;
		}
	}
	cout<<'\n';
}

int main(int argc, char **argv)
{
	std::map<char, double> charWeights; //Used to store each character with its weight
	std::map<char, string> encodedChars; //Used to assign binary code to each character in form of strings
	std::priority_queue<node*, vector<node*>, node_comp> pq; //This is the priority queue used to combine nodes and generate the root node of the tree
	string encMsg; //Final encoded message
	string msg;
	mode md;
	char mdc;
	
	do
	{
		cout<<"Enter e/E for encode mode, d/D for decode mode: \n";
		cin>>mdc;
	}
	while (mdc!='e' && mdc!='E' && mdc!='d' && mdc!='D');
	
	md = (mdc == 'e' || mdc =='E') ? ENC : DEC; //set encode or decode mode based on input
	
	std::cin.ignore(INT_MAX, '\n'); //flush cin due to >> operator used before to prepare for getline
	
	if (md==ENC) //Encode mode
	{
		do
		{
			cout<<"Enter a message to be encoded\n";
			getline(cin ,msg);
		} 
		while (msg.length()<=1);
		
		calc_probas(msg, charWeights); //calculates probabilities of each character in the input message, results are stored in the charWeights map
		
		//Creates huffman tree and results in having only the root node in the priority queue
		tree_create(pq, charWeights);
		
		//Traverses the tree recursively and stores binary codes for each character in encodedChars map
		traverse_tree(pq.top(), "", encodedChars);

		//Display each character and its binary code
		for (const auto &elem : encodedChars)
		{
			cout<<elem.first<<"  "<<elem.second<<'\n';
		}
		
		//Encodes the message using binary codes inside encodedChars map
		encMsg = encode_msg(msg, encodedChars);
		cout<<"Huffman encoded message: "<<encMsg<<'\n';
		
		ofstream outFile("huffout.dat", ios::out);
		save_tree(pq.top(), outFile); //saves tree structure for later use in case of decoding
		outFile.close();
		
		cout<<"Huffman tree is saved in huffout.dat, this can be used in decode mode with the encoded message (binary encoded message) to decode the message.\n";
		
	}
	else if (md==DEC) //Decode mode
	{
		ifstream inFile("huffout.dat");
		string s, decMsg;
		if (!inFile)
		{
			cout<<"Error opening input file\n";
			return -1;
		}
		node *nd = nullptr;
		load_tree(nd, inFile, pq);
		inFile.close();
		cout<<"Input huff coded msg: \n";
		getline(cin, decMsg);
		cout<<"\nDecoded message: \n";
		decode_huff(pq.top(), decMsg);
	}
	else
	{
		cout<<"Error!\n";
		return -1;
	}
	//Frees allocated heap memory for the nodes
	if (!pq.empty())
		free_resources(pq.top());
	return 0;
}
#include <bits/stdc++.h>
#include <iostream>
using namespace std;
//forward Declartions
int hextoint(char c);
string inttohex(int a);
int gfmulinv(int a);
int bitwise(int a);
int gfmul(int a, int b);
int gfadd(int a, int b);
void cls(vector<int>& arr, int cnt);
void crs(vector<int>& arr, int cnt);
vector<vector<int>> keyExp(string key);
void shiftrows(vector<vector<int>>& arr, int type);
void mixcols(vector<vector<int>>& arr);
void invmixcols(vector<vector<int>>& arr);
int subword(int a, int check);
void rcon();
vector<vector<int>> matrix(string plaintext);
string AES(string key, string plaintext);
void printgrid(vector<vector<int>>& grid);
string cipher(vector<vector<int>>& grid);
//

//Global Variables
vector<vector<int>> sbox(16, vector<int>(16));
vector<vector<int>> invsbox(16, vector<int>(16));
vector<int> r(11, 1);
//

//S-box creation
void table(){
    for(int i = 0; i<16; i++){
        for(int j = 0; j<16; j++){
            int mulinv = 0x00;
            string curr = inttohex(i)+inttohex(j);
            int value = stoi(curr, nullptr, 16);
            if(i != 0 || j != 0){
                mulinv = gfmulinv(value);
            }
            mulinv = bitwise(mulinv);
            string inv = inttohex(mulinv);
            int nx = hextoint(inv[0]);
            int ny = hextoint(inv[1]);
            if(mulinv < 16){
                nx = 0;
                ny = hextoint(inv[0]);
            }
            sbox[i][j] = mulinv;
            invsbox[nx][ny] = value;
        }
    }
}

/*Conversion of multiplicative inverse
to mapped values in S-box*/
int bitwise(int a){
    string s = "00000000";
    string c = "11000110";
    int ind = 0;
    while(a > 0){
        if(a&1) s[ind] = '1';
        a >>= 1;
        ind++;
    }
    string temp = s;
    for(int i = 0; i<8; i++){
        int curr = (s[i]-'0')^(s[(i+4)%8]-'0')^(s[(i+5)%8]-'0')^(s[(i+6)%8]-'0')^(s[(i+7)%8]-'0')^(c[i]-'0');
        if(curr) temp[i] = '1';
        else temp[i] = '0';
    }
    reverse(temp.begin(), temp.end());
    return stoi(temp, nullptr, 2);
}

//hex to int value
int hextoint(char c){
    if(c >= '0' && c <= '9') return c-'0';
    return c-'a'+10;
}

//int to hex value
string inttohex(int a){
    stringstream s;
    s << hex << a;
    string result(s.str());
    return result;
}

//Galois Field addition
int gfadd(int a, int b){
    return a^b;
}

//Galois Field multiplication
int gfmul(int a, int b){
    int mod = 0x1b;
    int p = 0;
    while(b>0){
        if(b&1){
            p ^= a;
            p &= 0xff;
        }
        bool highbit = (a&0x80);
        a<<=1;
        if(highbit){
            a ^= mod;
        }
        a &= 0xff;
        b>>=1;
    }
    return p;
}

//Calculating degree of each byte
int degree(int a){
    int ans = 0;
    a>>=1;
    while(a != 0){
        a >>= 1;
        ans += 1;
    }
    return ans;
}

//Calculating multiplicative inverse
int gfmulinv(int a){
    int v = 0x1b;
    int g1 = 1;
    int g2 = 0;
    int j = degree(a)-8;

    while(a != 1){
        if(j<0){
            swap(a,v);
            swap(g1,g2);
            j = -j;
        }
        a ^= (v<<j);
        g1 ^= (g2<<j);
        a &= 0xff;
        g1 &= 0xff;
        j = degree(a)-degree(v);
    }
    return g1;
}

//Circular right shift
void crs(vector<int>& arr, int cnt){
    vector<int> temp = arr;
    for(int i = 0; i<4; i++){
        arr[i] = temp[(i+4-cnt)%4];
    }
}

//Cicular left shift
void cls(vector<int>& arr, int cnt){
    vector<int> temp = arr;
    for(int i = 0; i<4; i++){
        arr[i] = temp[(i+cnt)%4];
    }
}

//Key Expansion Algoritm
vector<vector<int>> keyExp(string key){
    vector<vector<int>> word(44, vector<int>(4, 0));
    vector<int> temp;

    for(int i = 0; i<4; i++){
        word[i] = {stoi(key.substr(8*i,2), nullptr, 16), stoi(key.substr(8*i+2,2), nullptr, 16), 
            stoi(key.substr(8*i+4,2), nullptr, 16), stoi(key.substr(8*i+6,2), nullptr, 16)};
    }

    for (int i = 4; i < 44; i++){
        temp = word[i-1];
        if(i%4 == 0){
            cls(temp, 1);
            for(int j = 0; j<4; j++){
                temp[j] = subword(temp[j], 0);
                if(j == 0) temp[j]^=r[i/4];
            }
        }
        word[i] = word[i-4];
        for(int j = 0; j<4; j++) word[i][j] ^= temp[j];
    }
    return word;
}

//Shift Rows
void shiftrows(vector<vector<int>>& arr, int type){
    for(int i = 0; i<4; i++){
        if(type){
            cls(arr[i], i);
        }
        else{
            crs(arr[i], i);
        }
    }
}

//Mix Columns
void mixcols(vector<vector<int>>& arr){
    for(int i = 0; i<4; i++){
        vector<int> temp = {arr[0][i], arr[1][i], arr[2][i], arr[3][i]};
        arr[0][i] = gfmul(2,temp[0])^gfmul(3,temp[1])^temp[2]^temp[3];
        arr[1][i] = temp[0]^gfmul(2,temp[1])^gfmul(3,temp[2])^temp[3];
        arr[2][i] = temp[0]^temp[1]^gfmul(2,temp[2])^gfmul(3,temp[3]);
        arr[3][i] = gfmul(3,temp[0])^temp[1]^temp[2]^gfmul(2,temp[3]);
    }
}

//Inverse mix columns
void invmixcols(vector<vector<int>>& arr){
    for(int i = 0; i<4; i++){
        vector<int> temp = {arr[0][i], arr[1][i], arr[2][i], arr[3][i]};
        arr[0][i] = gfmul(0x0e,temp[0])^gfmul(0x0b,temp[1])^gfmul(0x0d,temp[2])^gfmul(0x09,temp[3]);
        arr[1][i] = gfmul(0x09,temp[0])^gfmul(0x0e,temp[1])^gfmul(0x0b,temp[2])^gfmul(0x0d,temp[3]);
        arr[2][i] = gfmul(0x0d,temp[0])^gfmul(0x09,temp[1])^gfmul(0x0e,temp[2])^gfmul(0x0b,temp[3]);
        arr[3][i] = gfmul(0x0b,temp[0])^gfmul(0x0d,temp[1])^gfmul(0x09,temp[2])^gfmul(0x0e,temp[3]);
    }
}

//Substituting byte using s-box
int subword(int a, int check){
    string s = inttohex(a);
    int nx = hextoint(s[0]);
    int ny = hextoint(s[1]);
    if(a < 16){
        nx = 0;
        ny = hextoint(s[0]);
    }
    if(check) return invsbox[nx][ny];
    return sbox[nx][ny];
}

//Rcon vector using in key expansion
void rcon(){
    for(int i = 2; i<11; i++){
        r[i] = (2*r[i-1])%229;
    }
}

//converting string to matrix form
vector<vector<int>> matrix(string plaintext){
    vector<vector<int>> grid(4, vector<int>(4));
    for(int i = 0; i<4; i++){
        grid[0][i] = stoi(plaintext.substr(8*i,2), nullptr, 16);
        grid[1][i] = stoi(plaintext.substr(8*i+2,2), nullptr, 16);
        grid[2][i] = stoi(plaintext.substr(8*i+4,2), nullptr, 16);
        grid[3][i] = stoi(plaintext.substr(8*i+6,2), nullptr, 16);
    }
    return grid;
}

//Add round key function
void rndkey(vector<vector<int>> word, vector<vector<int>>& grid, int rnd){
    for (int i = 0; i < 4; i++){
        grid[0][i] ^= word[rnd*4+i][0];
        grid[1][i] ^= word[rnd*4+i][1];
        grid[2][i] ^= word[rnd*4+i][2];
        grid[3][i] ^= word[rnd*4+i][3];
    }
    
}

//Encryption Algo
string Enc(string key, string plaintext){
    vector<vector<int>> word = keyExp(key);
    vector<vector<int>> grid = matrix(plaintext);
    rndkey(word, grid, 0);
    for(int j = 1; j<11; j++){
        for(int i = 0; i<4; i++){
            grid[i][0] = subword(grid[i][0], 0);
            grid[i][1] = subword(grid[i][1], 0);
            grid[i][2] = subword(grid[i][2], 0);
            grid[i][3] = subword(grid[i][3], 0);
            cls(grid[i], i);
        }
        if(j != 10 )mixcols(grid);
        rndkey(word, grid , j);
    }
    return cipher(grid);
}

//Matrix to string conversion
string cipher(vector<vector<int>>& grid){
    string cipher = "";
    for(int i = 0; i<4; i++){
        for(int j = 0; j<4; j++){
            string temp = inttohex(grid[j][i]);
            if(temp.length() == 1) cipher += '0';
            cipher += temp;
        }
    }
    return cipher;
}

//Printing the matrix
void printgrid(vector<vector<int>>& grid){
    for(int i = 0; i<4; i++){
        for(int j = 0; j<4; j++){
            cout<<hex<<grid[i][j]<<" ";
        }
        cout<<endl;
    }
    cout<<endl<<endl;
}

//Decryption Algo
string Dec(string ciphertext, string key){
    vector<vector<int>> word = keyExp(key);
    vector<vector<int>> grid = matrix(ciphertext);
    rndkey(word, grid, 10);
    for(int i = 9; i>=0 ; i--){
        for(int i = 0; i<4; i++){
            grid[i][0] = subword(grid[i][0], 1);
            grid[i][1] = subword(grid[i][1], 1);
            grid[i][2] = subword(grid[i][2], 1);
            grid[i][3] = subword(grid[i][3], 1);
            crs(grid[i], i);
        }
        rndkey(word, grid, i);
        if(i != 0) invmixcols(grid);
    }
    return cipher(grid);
}

int main(){
    string key = "0f1571c947d9e8590cb7add6af7f6798";
    string plaintext = "0123456789abcdeffedcba9876543210";
    rcon();
    table();
    string ciphertext = Enc(key, plaintext);
    cout<<ciphertext<<endl;
    string decryption = Dec(ciphertext, key);
    cout<<decryption;
    return 0;
}
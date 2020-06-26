#include <math.h>
#include <String.h>
//#include <sstream>
#include <stdlib.h>
# include <stdio.h>
//#include <String.h>

using namespace std;

struct funcReturn{
        int length;
        unsigned char* data;
};

String dec2hex(int input);
int hexString2int(String input);
funcReturn txSMS(char* phonenumber, char* packetData1);
void print_as_dec(unsigned char* data,int length);
void print_as_hex(unsigned char* data,int length);
/**
int main()
{
    char packetData[160] = "131717,39.6967,-78.1957,172,test";//"131717,39.6967,-78.1957,172,test";
    char phonenumber[12] = "7324849689";

    funcReturn hexCode = txSMS(phonenumber, packetData);
    //print_as_dec(hexCode.data,hexCode.length);
    //cout << '\n' <<endl;
    //print_as_hex(hexCode.data,hexCode.length);
    //cout << endl;
}
*/

funcReturn txSMS(char* phonenumber, char* packetData){
    String outputData[200];
    String startdelim = "7E";
    int length = 0;
    String frameType = "1F";
    String frameId = "01";
    String options = "00";
    length = 3;
    int sum = 32;//1+31 (10 + 1F)
    unsigned char finalData[200];
    memset(finalData, 0, 200);


    //cout<<"Chartest: "<<int("7")<<" Words"<<endl;


    length += 20;
    String number[22];
    int counter = 0;
    for(int i=0;i<20;i++){
        if(counter<strlen(phonenumber)){
            number[counter] = dec2hex(phonenumber[i]);
            sum+=int(phonenumber[i]);
        }else{
            number[counter] = "00";
        }

        counter++;
    }

    //char packetData[160] = "x";//"131717,39.6967,-78.1957,172,test";
    length+=strlen(packetData);

    int packetDataLength = strlen(packetData);

    counter = 0;

    for(int i = 0;i<packetDataLength;i++){
        outputData[i] = dec2hex(packetData[i]);
        sum+=packetData[i];
    }

    /**
    for(int x : packetData){
        outputData[counter] = dec2hex(x);
        counter++;
        sum+=x;
    }
    */

    //cout<<endl<<"SUM: "<<sum<<" "<<dec2hex(sum)<<endl;
    String pchecksum = dec2hex(sum);
    String check = pchecksum.substr(pchecksum.length()-2,2);
    char char_array[3];
    strcpy(char_array, check.c_str());
    int decval = 255-(int)strtol(char_array, NULL, 16);
    check = dec2hex(decval);
    String checksum="";
    if(decval<16){
        checksum.append("0");
    }
    checksum.append(check);

    //cout<<"Length: "<<length<<endl;
    String nullLength = "00";
    String packetLength = dec2hex(length);

    //cout <<"Length: "<<nullLength<<" "<<packetLength<<endl;

    //cout<<"Whole Packet:"<<endl;
    //cout<<startdelim<<" "<<nullLength<<" "<<packetLength<<" "<<frameType<<" "<<frameId<<" "<<options<<" ";

    counter = 0;
    for(String i:number){
        //cout << i <<" ";
        finalData[6+counter] = hexString2int(i);
        counter++;
    }

    //cout << packetDataLength <<endl;

    counter = 0;
    for(int iter = 0;iter<packetDataLength;iter++){
        //cout << outputData[iter] <<" ";
        finalData[26+counter] = hexString2int(outputData[iter]);
        counter++;
    }
    //cout<<checksum<<endl;

    //cout << hexString2int(startdelim)<<" "<<hexString2int(packetLength)<<" "<<hexString2int(frameType)<<" "<<hexString2int(frameId)<<" "<<hexString2int(options)<<endl;

    finalData[0] = hexString2int(startdelim);
    finalData[1] = 0;
    finalData[2] = hexString2int(packetLength);
    finalData[3] = hexString2int(frameType);
    finalData[4] = hexString2int(frameId);
    finalData[5] = hexString2int(options);
    finalData[26+counter] = hexString2int(checksum);

    int totalLength = 27+counter;

    /**
    for(int i = 0;i<totalLength;i++){
        printf("%x,",finalData[i]);
    }


    cout << '\n' << endl;




    for(int i = 0;i<totalLength;i++){
        printf("%u,",finalData[i]);
    }

    cout <<'\n'<<endl;
    */

    funcReturn out;
    out.length = totalLength;
    out.data = finalData;
    return out;
}

void print_as_hex(unsigned char* data,int length){
    for(int i = 0;i<length;i++){
        printf("%x,",data[i]);
    }
}

void print_as_dec(unsigned char* data,int length){
    for(int i = 0;i<length;i++){
        printf("%u,",data[i]);
    }
}

int hexString2int(String input){

    int output;
    /**
    std::Stringstream ss;
    ss << std::hex << input;
    ss >> output;
    */

    
    //return std::stoi(output,0,16);
    return output;
}

String dec2hex(int input){
    int r;
    char hex[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    String hexdec_num="";
    while(input>0)
    {
        r = input % 16;
        hexdec_num = hex[r] + hexdec_num;
        input = input/16;
    }

    return hexdec_num;
}

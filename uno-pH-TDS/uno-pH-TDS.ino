#define DEBUG	0

//TDS variables
#define TdsSensorPin A1
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;
int getMedianNum(int bArray[], int iFilterLen);


//pH variables
#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;
double avergearray(int* arr, int number);


#define update_data_server 5000

void setup()
{
    Serial.begin(9600);
    pinMode(TdsSensorPin,INPUT);
	pinMode(LED,OUTPUT);
}

void loop()
{
	//TDS code
	static unsigned long analogSampleTimepoint = millis();
	if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
	{
		analogSampleTimepoint = millis();
		analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
		analogBufferIndex++;
		if(analogBufferIndex == SCOUNT) 
			analogBufferIndex = 0;
	}   
	static unsigned long printTimepoint = millis();
	if(millis()-printTimepoint > 800U)
	{
		printTimepoint = millis();
		for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
		analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
		averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
		float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
		float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
		tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
		#if DEBUG 
			Serial.print("voltage:"); 
			Serial.print(averageVoltage,2);
			Serial.print("V   "); 
			Serial.print("TDSValue:"); 
			Serial.print(tdsValue,0); 
			Serial.println("ppm"); 
		#endif
	}
	
	//pH code
	static unsigned long samplingTime = millis();
	static unsigned long printTime = millis();
	static float pHValue,voltage;
	if(millis()-samplingTime > samplingInterval)
	{
		pHArray[pHArrayIndex++]=analogRead(SensorPin);
		if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
		voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
		pHValue = 3.5*voltage+Offset;
		samplingTime=millis();
	}
	if(millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
	{
		#if DEBUG 
			Serial.print("Voltage:"); 
			Serial.print(voltage,2); 
			Serial.print("pHValue:"); 
			Serial.print(pHValue,2);
		#endif
		digitalWrite(LED,digitalRead(LED)^1);
		printTime=millis();
	}
	
	static unsigned long updateTime = millis();
	if(millis() - updateTime > update_data_server){
		Serial.print(tdsValue,0);
		Serial.print(";"); 
		Serial.print(pHValue,2);
		Serial.print(";"); 
		Serial.println("end"); 
	}
}

//TDS code
int getMedianNum(int bArray[], int iFilterLen) 
{
	int bTab[iFilterLen];
	for (byte i = 0; i<iFilterLen; i++)
		bTab[i] = bArray[i];
	int i, j, bTemp;
	for (j = 0; j < iFilterLen - 1; j++) 
	{
		for (i = 0; i < iFilterLen - j - 1; i++) 
		{
			if (bTab[i] > bTab[i + 1]) 
			{
				bTemp = bTab[i];
				bTab[i] = bTab[i + 1];
				bTab[i + 1] = bTemp;
			}
		}
	}
	if ((iFilterLen & 1) > 0)
		bTemp = bTab[(iFilterLen - 1) / 2];
	else
		bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
	return bTemp;
}

//pH code
double avergearray(int* arr, int number)
{
	int i;
	int max,min;
	double avg;
	long amount=0;
	if(number<=0){
		#if DEBUG 
			Serial.println("Error number for the array to avraging!/n");
		#endif
		return 0;
	}
	if(number<5){   //less than 5, calculated directly statistics
		for(i=0;i<number;i++){
			amount+=arr[i];
		}
		avg = amount/number;
		return avg;
	}else
	{
		if(arr[0]<arr[1]){
			min = arr[0];max=arr[1];
		}
		else
		{
			min=arr[1];max=arr[0];
		}
		for(i=2;i<number;i++){
			if(arr[i]<min){
				amount+=min;        //arr<min
				min=arr[i];
			}else 
			{
				if(arr[i]>max){
					amount+=max;    //arr>max
					max=arr[i];
				}else{
				amount+=arr[i]; //min<=arr<=max
				}
			}//else
		}//for
		avg = (double)amount/(number-2);
	}//else
	return avg;
}

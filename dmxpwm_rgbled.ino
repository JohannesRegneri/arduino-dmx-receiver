/*              
The Code doesn´t look very good or is easy to understand.
This version is just a big mess of Code from diffrent Sources.
Basic structure is from https://www.crazy-logic.co.uk/projects/lighting/receiving-dmx-on-arduino-with-ardmx

The was tested on Arduino Nano and UNO.

If you want to change it for your preferences
change:
      
     #define Dmxchannels 6
     DmxAddress = 48;
     Outputs in Setup
     Void action
     
                
              
              */
              
              
              
              
              
              
              
              /* 
              pinout 
               pin0 - serial in
               pin1
               pin2  ch1 indicator
               pin3~
               pin4  Switch ch 7 +127 8 +127
               pin5~
               pin6~
               pin7 - error2 - bad dmx (non dimmer data)
               pin8 - error3 - last channel not recieved. 
               pin9~   R-
               pin10~  G-
               pin11~  B-
               pin12 - error1 - tooo much time between frames. 
               pin13 - dmx data output for demo. ch2 ind
               apin0
               apin1
               apin2
               apin3
               apin4
               apin5
               pinout 
              
                                                                                            ***INFO*** 
                                                                
                                                                LED 1 Pin to 5V; 9(R), 11(G), 10(B), LOW  -> LED of Channelbyte 255 (HIGH)
                                                                -3 Pin extra channel; 3, 5, 6, 
                                                                
                                                                Startadress 13
                                                                
                                                                      -Next feature 
                                                                              -Main Dimmer Channel 
                                                                              -4ch Mode
                                                                              - + Shutter 5ch
              */  
//***************************************************************************************************************************************************************************************************************************              

int PwmOut[6]={6, 3, 5, 9, 10, 11};



              
              #define Dmxchannels 6          //the number of channels we want.                                                                 3
              volatile uint8_t  DmxRxField[Dmxchannels]; //array of 8 bit int DMX vals (raw)
              volatile uint16_t DmxAddress;  //start address (16 bit int)
              
              enum States {
                IDLE, BREAK, STARTCODE, MYADDRESSES}; 
                States gDmxState;             //create a global variable. 
                                              //idle - nothing interesting to be dooing, 
                                              //break - the break has been recieved so expect some data after a startcode. 
                                              //startcode - the startcode wase recieved so we're now into the frames of data stage. 
                                              //myaddresses - we've started receieving data for my addresses. 
                                              
                                              //error1 is no new packet within time defined by the defualt timer. 
              boolean error1 = true;
              int timer;
              int defaulterror1time=500;
                                              // i recomend at least 100 to avoid the LED flickering between packets, 
                                              // and not more than 2000, as 2 seconds is reasonably long. 
                                              
                                              //error2 for bad DMX (non dimmer data)
              boolean error2 = true;
                                              //error 2 is all to do with the valid startcode.... and it only apperas in that code. 
                                              
                                              //error3 is for last channel not recieved. 
                                              //it only is true if a new packet is recieved before the end of the last one. 
                                              //its implimented in two parts. 
                                              //serror3 is within the ISR, it's set to true at the begining of a packet, 
                                              //serror3 is set to false once the last frame expected is recieved. 
              boolean error3 = false;
              boolean serror3 = false;
                                              //in the break section of code, if it gets to a break and is still an error 
                                              //then that means that error3 has occoured.  
                                              
//****************************************************************************************************************************************************************************************************************************                                              
                        
              void setup()
              {
                Serial.begin(250000);                  //Enable serial reception with a 250k rate
                gDmxState= IDLE;                       // initial state
                DmxAddress = 48;                        // The desired DMX Start Adress                                                                    4
                timer=defaulterror1time;
                
                analogWrite(PwmOut[1], 255);
                analogWrite(PwmOut[2], 255);
                analogWrite(PwmOut[3], 255);
                pinMode(3, OUTPUT);
                pinMode(5, OUTPUT);
                pinMode(6,OUTPUT);
                pinMode(9, OUTPUT);
                pinMode(10, OUTPUT);
                pinMode(11,OUTPUT);
              }
              
 
  
  
  
  //***************************************************************************************************************************************************************************************************************************
              
              ISR(USART_RX_vect) {
                                                  //this code/function is called whenever the harware UART recieves a low 
                                                  //startbit (well it's called after the byte is recieved.)
                                                
                                                  // two byte counter - needed to count upto all 512 channel frames
                static  uint16_t DmxCounter;
              
                uint8_t  USARTstate= UCSR0A;      //get state before data!
                uint8_t  DmxByte   = UDR0;	    //get data
                uint8_t  DmxState  = gDmxState;	  //just load once from SRAM to increase speed
              
                if (USARTstate &(1<<FE0))		//check for break
                {
                                                    //data has been recieved, the USART is enabled and it's 
                                                    //low signal for more than 8 bits
                                                    // so an error has also been created - this is a Break. 
                  gDmxState= BREAK;
                                                    //as this marks the start of a new packet we shall set a counter to the 
                                                    //address we wish to use as our start address.
                  DmxCounter =  DmxAddress; 
                                                    //dmx counter counts down untill it reaches the address	
                  if(serror3==true) {
                    error3=true;
                  }	
                  serror3 = true;                   //as we're at the start of a packet.
                                                    //lets set the error1 timer back to it's default and set it to 
                                                    //false as DMX is imcomming 
                  timer=defaulterror1time; 
                  error1=false; 	
                }
              
                                                  //lets assume that the break has occoured - then to get this far in the code, 
                                                  //another frame of data has been recieved. As the //break has occoured then the 
                                                  //next frame should be the start code. The start code for DMX data for 
                                                  //dimmer channels is the 0 value.  
                else if (DmxState == BREAK)
                {
                  if (DmxByte == 0) //normal start code detected
                  {
                    gDmxState= STARTCODE; 
                    error2=false;
                  }  
                  else	                            //non normal start code recieved eg: RDM packet. 
                                                    //ignore the rest of the packet.
                  { 
                    gDmxState= IDLE; 
                    error2=true;
                  } 
                }
              
                                                    //to get to this point, we've had a break, then the start code. 
                                                    //the next frame should be the the first channel of DMX data. 
                else if (DmxState == STARTCODE)
                {
                                                    //now we need to see if we are in the data before the start address 
                                                    //or at the start address.
                                                    //to do this lets decrement the counter and see if it's 0. 
                                                    //the counter initially was set to the start address, so when it gets to 
                                                    //the start address's frame it should be zero. 
                                                    
                  if (--DmxCounter == 0)	    //start address reached?
                  {
                                                    //wahoo - we're at the start address so lets grab this frame
                    DmxRxField[DmxCounter]= DmxByte;	//get 1st DMX channel of device
                    DmxCounter= 1;		//set up counter for required channels
                    if(Dmxchannels==1){
                      gDmxState= IDLE;
                      serror3 = false;
                    }
                    else{
                      gDmxState= MYADDRESSES;
                    }
                    //
                  }
                }
              
                                                    //to get to here we've recieved a break, start code and frams upto and 
                                                    //including the first channel we want.  
                else if (DmxState == MYADDRESSES)
                {
                                                    //lets incriment the counter, 
                                                    //is the new counter equal to the number of channels we require?
                                                    //(remember here that the DMX counter is zero indexed and the 
                                                    //Dmx channels is 1 indexed...
                                                    //if so then we've gone past our last channel so go back to the idel 
                                                    //state.
                                                    //if not then this data is for one of our channels so chuck it into 
                                                    //the array.  
                  DmxRxField[DmxCounter++]= DmxByte;	//get channel
                  if (DmxCounter >= Dmxchannels)     //all ch received?
                  {
                    gDmxState= IDLE;	            //wait for next break  
                    serror3 = false;               //as we're at the end of data expected. 
                  }
                }
                //end of ISR	
              }
              
//***************************************************************************************************************************************************************************************************************************      
              
              void loop()
              {
                action();
              
                                                    
                delay(1); 
                                                    //this delays the loop by 1ms... less than ideal but required for the timer
                                                    //counting down between successfull packets.
                                                    //if your action function takes significant time to output stuff then 
                                                    //you could remove the delay by making it a comment as the action loop 
                                                    //would be a time delay. in this case the timer is then more of a counter, 
                                                    //with error 1 being a number of action calls without a successfull DMX 
                                                    //packet being recieved. 
              }
              
//***************************************************************************************************************************************************************************************************************************                            
            
              
              void action()
              {
                
                            
                for (int i = 0 ; i < 6 ; i++)
                {
                  if(DmxRxField[i] < 1) {
                    analogWrite(PwmOut[i], 255);
                  }
                  else {
                    analogWrite(PwmOut[i],-DmxRxField[i]);
                }

              }
              
             
              }
              
              
              
              
              
              
              
              
              
              
              


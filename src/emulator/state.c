#include "state.h"
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

State* newState(Memory* memory) {
    State* s =(State*) malloc(sizeof(State));
    s->mem = memory;
    for(int i=0;i<32;i++){
        s->gprs[i]=0;
    }
    for(int i=0;i<3;i++){
        s-> cr[i]=0;
    }
    return s;
}

uint32_t extract(uint32_t total,int start,int end){
    uint32_t temp = total << start;
    //uint32_t cutoff = (uint32_t) temp;
    temp = temp >> (31-(end-start));
    return temp;
}

void statedump(State* s){
    for(int i=0;i<32;i++){
        printf("GPR%d:%lx\n",i,s->gprs[i]);
    }
    for(int i=0;i<3;i++){
        printf("CR%d:%lx\n",i,s->cr[i]);
    }
    printf("PC:%lx\n",s->pc);
    printf("LR:%lx\n",s->lr);
}

void run(State* s) {
    int run =1;//186
    while(run/*<670*/){
        uint32_t check = read32(s->mem,s->pc); 
        uint32_t op = extract(check,0,5);
	/*if(s->pc == 0x10000188){
		printf("RUNERUANURENURNUSERN:%d\n",run);
	}*/
        //printf("RUN:%d\n",run);
        //printf("PC:%lx\n",s->pc);
	//printf("OP:%lx\n",(int64_t)op);
        //printf("\n");
        switch(op){
            case 11:{//cmpi
                        uint32_t RA11 = extract(check,11,15);
                        uint32_t SI11 = extract(check,16,31);
                        uint32_t temp11 = s->gprs[RA11];
                        if(temp11 < SI11){
                            s->cr[0]=1;
                            s->cr[1]=0;
                            s->cr[2]=0;
                        }
                        else if(temp11 > SI11){
                            s->cr[0]=0;
                            s->cr[1]=1;
                            s->cr[2]=0;
                        }
                        else{
                            s->cr[0]=0;
                            s->cr[1]=0;
                            s->cr[2]=1;
                        }
                        s->pc+=4;
                        run++;
                        break;
                    }

            case 14:{//addi and li
                        uint32_t RT14 = extract(check,6,10);
                        uint32_t RA14 = extract(check,11,15);
                        uint32_t SI14 = extract(check,16,31);
                        SI14= SI14 << 16;
                        int32_t extend = (int32_t)SI14>>16;
                        if(RA14==0){
                            //s->gprs[RT14] = s->gprs[RA14] + extend;
			    s->gprs[RT14] = extend;
                        }
                        else{
                            //s->gprs[RT14] = extend;
			    s->gprs[RT14] = s->gprs[RA14] + extend;
                        }
                        s->pc+=4;
                        run++;
                        break;
                    }

            case 16:{//beq(bc)
                        uint32_t BO16 = extract(check,6,10);
                        //uint32_t BI16 = extract(check,11,15);
                        uint32_t tempextend = extract(check,16,29);
                        tempextend=tempextend<<2;
                        tempextend=tempextend<<16;
                        int32_t extend = (int32_t) tempextend>>16;
                        if(BO16==4){
                            if(!(s->cr[2])){
                                s->pc += extend;
                            }
                            else{
                                s->pc+=4;
                            }
                        }
                        else{
                            if(s->cr[2]){
                                s->pc+=extend;
                            }
                            else{
                                s->pc+=4;
                            }
                        }
                        run++;
                        break;
                    }

            case 17:{//sc
                        int callvalue = s->gprs[0];
                        if(callvalue == 1){
                            run = 0;
                        }
                        if(callvalue == 4){
                            /*if(s->gprs[4]==s->gprs[10]){
                              printf("%c\n",read8(s->mem,s->gprs[4]));
                              }
                              else{*/
                            printf("%c",read8(s->mem,s->gprs[4]));
                            //}
                            run++;
                            s->pc += 4;
                        }
                        break;
                    }

            case 18:{//b and bl
                        uint64_t LI =0;
                        //statedump(s);
                        //uint64_t NIA = 0;
                        LI = extract(check,6,29);
                        /*printf("LI:%lx\n",LI);
                          printf("PC:%lx\n",s->pc);*/
                        LI = LI << 2;
                        LI = LI << 6;
                        int32_t LISE= (int32_t)LI>>6;
                        //NIA = s->pc - (0x50);
                        uint32_t flag = extract(check,31,31);
                        if(flag){
                            s->lr = s->pc +4;
                        }
                        s->pc += LISE;
                        run++;
                        break;
                    }
            case 19:{//blr
                        s->pc = s->lr;
                        break;
                    }
            case 21:{//srwi(rlwinm)
                        uint32_t RA = extract(check,6,10);
                        uint32_t RS = extract(check,11,15);
                        int32_t MB = extract(check,21,25);
			//MB = MB << 27;
			//int32_t extend = (int32_t) MB >> 27;
                        s->gprs[RA] = s->gprs[RS] >> MB;
                        s->pc+=4;
			run++;
                        break;
                    }

            case 24:{//ori
                        uint64_t RS24 = extract(check,6,10);
                        uint64_t RA24 = extract(check,11,15);
                        uint64_t UI24 = extract(check,16,31);
                        //printf("UI24:%lu\n",UI24);
                        s->gprs[RA24] = s->gprs[RS24]|UI24;
                        s->pc+=4;
                        run++;
                        break;
                    }

            case 25:{//oris
                        uint64_t RS25 = extract(check,6,10);
                        uint64_t RA25 = extract(check,11,15);
                        uint64_t UI25 = extract(check,16,31);
                        UI25 = UI25 << 16;
                        s->gprs[RA25] = (s->gprs[RS25])|UI25;
			//s->gprs[RA25] = s->gprs[RA25] >> 16;
                        s->pc+=4;
                        run++;
                        break;
                    }

            case 31:{
                        uint64_t temp = extract(check,21,30);
                        switch(temp){
                            uint64_t RS;
                            uint64_t RA;
                            uint64_t RB;
                            uint64_t RT;
                            case 0:{//cmp
                                        RA = extract(check,11,15);
                                        RB = extract(check,16,20);
                                        int a = s->gprs[RA];
                                        int b = s->gprs[RB];
                                        if(a<b){
                                            s->cr[0] = 1;
                                            s->cr[1] = 0;
                                            s->cr[2] = 0;
                                        }
                                        else if (a>b){
                                            s->cr[0] = 0;
                                            s->cr[1] = 1;
                                            s->cr[2] = 0;
                                        }
                                        else{
                                            s->cr[0] = 0;
                                            s->cr[1] = 0;
                                            s->cr[2] = 1;
                                        }
					break;
                                   }
                            case 19:{//mfcr
                                        RT = extract(check,6,10);
					uint32_t temp = 0;
					uint32_t placeholder = 1;
					for(int i = 0; i < 3; i ++){
					    for(int x =0; x<2-i;x++){
					    	placeholder*=10;
					    }
					    temp += placeholder*s->cr[i];
					    placeholder = 1;
					}
					s->gprs[RT] = temp << 29;
					run++;
                                        //uint32_t regcr = s->gprs[RT];
					break;
                                        
                            }
                            case 28:{//and
                                        RS = extract(check,6,10);
                                        RA = extract(check,11,15);
                                        RB = extract(check,16,20);
                                        s->gprs[RA] = s->gprs[RS] & s->gprs[RB];
                                        break;
                                    }

                            case 40:{//subf
                                        RT = extract(check,6,10);
                                        RA = extract(check,11,15);
                                        RB = extract(check,16,20);
                                        s->gprs[RA]=s->gprs[RB] - s->gprs[RA];
                                        run++;
                                        break;
                                    }

                            case 215:{//stbx
                                         uint64_t RS215 = extract(check,6,10);
                                         uint64_t RA215 = extract(check,11,15);
                                         uint64_t RB215 = extract(check,16,20);
                                         /*uint64_t EA =0;
                                           if(RA215){
                                           EA = (int64_t)s->gprs[RA215] +(int64_t)s->gprs[RB215];
                                           }
                                           else{
                                           EA = (int64_t)s->gprs[RB215];
                                           }
                                           uint8_t towrite = s->gprs[RS215];
                                           write8(s->mem,EA,towrite);*/
                                         if(RA215==0){
                                             //write8(s->mem,s->gprs[RB215]+s->gprs[RA215],(uint8_t)s->gprs[RS215]);
					     write8(s->mem,s->gprs[RB215],(uint8_t)s->gprs[RS215]);
                                         }
                                         else{
                                             //write8(s->mem,s->gprs[RB215],(uint8_t)s->gprs[RS215]);
					     write8(s->mem,s->gprs[RB215]+s->gprs[RA215],(uint8_t)s->gprs[RS215]);
                                         }
                                         run++;
                                         break;
                                     }

                            case 233:{//mulld
                                         RT = extract(check,6,10);
                                         RA = extract(check,11,15);
                                         RB = extract(check,16,20);
                                         s->gprs[RT]=(s->gprs[RA])*(s->gprs[RB]);
                                         run++;
                                         break;
                                     }

                            case 266:{//add
                                         RT = extract(check,6,10);
                                         RA = extract(check,11,15);
                                         RB = extract(check,16,20);
                                         s->gprs[RT] = s->gprs[RA] + s->gprs[RB];
                                         run++;
                                         break;
                                     }

                            case 316:{//xor
                                         //statedump(s);
                                         RS = extract(check,6,10);
                                         RA = extract(check,11,15);
                                         RB = extract(check,16,20);
                                         s->gprs[RA]=(s->gprs[RS]^s->gprs[RB]);
                                         run++;
                                         break;
                                     }

                            case 339:{//mflr
                                         //statedump(s);
                                         uint64_t reg = extract(check,6,10);
                                         s->gprs[reg]=s->lr;
                                         run++;
                                         break;
                                     }

                            case 444:{//or
                                         //statedump(s);
                                         RS = extract(check,6,10);
                                         RA = extract(check,11,15);
                                         RB = extract(check,16,20);
                                         s->gprs[RA]=(s->gprs[RS]|s->gprs[RB]);
                                         run++;
                                         break;
                                     }

                            case 457:{//divdu
                                         RT = extract(check,6,10);
                                         RA = extract(check,11,15);
                                         RB = extract(check,16,20);
                                         s->gprs[RT] = (s->gprs[RA])/(s->gprs[RB]);
                                         run++;
                                         break;
                                     }

                            case 467:{//mtlr
                                         RT = extract(check,6,10);
                                         s->lr = s->gprs[RT];
                                         break;
                                     }
                        }
                        s->pc+=4;
                        break;
                    }

            case 38:{
                        uint32_t RS38 = extract(check,6,10);
                        uint32_t RA38 = extract(check,11,15);
                        uint32_t D38 = extract(check,16,31);
                        D38 = D38 << 16;
                        int32_t extend = (int32_t) D38>>16;
                        if(RA38==0){
                            write8(s->mem,extend,(uint8_t)s->gprs[RS38]);
                        }
                        else{
			    write8(s->mem,extend+s->gprs[RA38],(uint8_t)s->gprs[RS38]);
                        }
                        s->pc +=4;
                        run++;
                        break;
                    }

            case 58:{//ld
                        uint32_t RT58 = extract(check,6,10);
                        uint32_t RA58 = extract(check,11,15);
                        uint32_t DS58 = extract(check,16,29);
                        DS58 = DS58<<2;
                        DS58 = DS58<<16;
                        int32_t extended = (int32_t)DS58>>16;
                        if(RA58==0){
                            //s->gprs[RT58] = read64(s->mem,s->gprs[RA58]+extended);
			    s->gprs[RT58] = read64(s->mem,extended);
                        }
                        else{
                            //s->gprs[RT58] = read64(s->mem,extended);
			    s->gprs[RT58] = read64(s->mem,s->gprs[RA58]+extended);
                        }
                        s->pc+=4;
                        run++;
                        break;
                    }

            case 62:{//std and stdu
                        //statedump(s);
                        uint32_t RS62 = extract(check,6,10);
                        uint32_t RA62 = extract(check,11,15);
                        uint32_t DS62 = extract(check,16,29);
                        DS62 = DS62<<2;
                        DS62 = DS62<<16;
                        int32_t extended = (int32_t) DS62>>16;
                        write64(s->mem,s->gprs[RA62]+extended,s->gprs[RS62]);
                        if(extract(check,30,31)){
                            s->gprs[RA62] = s->gprs[RA62]+extended;
                        }
                        s->pc+=4;
                        run++;
                        break;
                    }

            default:
                    break;

        }
    }
    //statedump(s);
}

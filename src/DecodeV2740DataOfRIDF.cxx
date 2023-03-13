#include "../include/DecodeV2740DataOfRIDF.h"

int main(int argc, char** argv){
  TArtEventStore *estore = new TArtEventStore();
  estore->Open(argv[1]);
  TArtRawEventObject *rawEvent = estore->GetRawEventObject();
  
  TFile *fout = TFile::Open("DecodeV2740DataOfRIDF.root", "RECREATE");
  TTree *tree = new TTree("tree", "tree");
  const int NCH = 64;
  int fDoNotCutBranchYet = true;
  int Nsamples;
  boost::multi_array<int,      1> timeBacket;
  boost::multi_array<int,      2> fadc;
  boost::multi_array<int16_t,  3> analogProbe;
  boost::multi_array<uint8_t,  3> digitalProbe;  
  boost::multi_array<uint8_t,  2> analogProbeInfo, digitalProbeInfo, analogMultiFac;
//  boost::multi_array<int16_t,  2> analogProbe;
//  boost::multi_array<uint8_t,  2> digitalProbe;
//  boost::multi_array<uint8_t,  1> analogProbeInfo, digitalProbeInfo;
  boost::multi_array<uint16_t, 1> energy;  
  vector<int> chEnabled;
  uint64_t tsFADC;


  tree->Branch("TSFADC",           &tsFADC,          "TSFADC/l");

  uint64_t counter=0;  
  while(estore->GetNextEvent()){
//  for(int k=0;k<10;k++){
//    estore->GetNextEvent();
    
    int fFill = true;
    for(int iseg=0;iseg<rawEvent->GetNumSeg();iseg++){
      TArtRawSegmentObject *seg = rawEvent->GetSegment(iseg);
      int detector = seg->GetDetector();
      if(detector == 0){
	TArtRawV2740DataObject *ftemp = seg->GetV2740Data(0);
	int tbtemp   = ftemp->GetTimeBacket(0);
	int fadctemp = ftemp->GetVal(0);
	if( (tbtemp==-1) && (fadctemp==-1) ){
	  cout<<"DecodeV2740DataOfRIDF : Skip start run!"<<endl;
	  fFill = false;
	  break;
	}

	if(fDoNotCutBranchYet){
	  Nsamples = ftemp->GetNData();
	  //	  cout<<"Nsamples = "<<Nsamples<<endl;
	  timeBacket      .resize(boost::extents[Nsamples]);
	  fadc            .resize(boost::extents[NCH][Nsamples]);
	  analogProbe     .resize(boost::extents[NCH][2][Nsamples]);
	  digitalProbe    .resize(boost::extents[NCH][4][Nsamples]);
	  analogProbeInfo .resize(boost::extents[NCH][2]);
	  digitalProbeInfo.resize(boost::extents[NCH][4]);
	  analogMultiFac  .resize(boost::extents[NCH][2]);
	  energy          .resize(boost::extents[NCH]);	  

	  stringstream ss;
	  ss.str(""); ss<<"TimeBacket["<<Nsamples <<"]/I";                tree->Branch("TimeBacket",       timeBacket      .origin(), ss.str().c_str());
	  ss.str(""); ss<<"fadc["      <<NCH<<"]["<<Nsamples<<"]/I";      tree->Branch("fadc",             fadc            .origin(), ss.str().c_str());
	  ss.str(""); ss<<"AnalogProbe["<<NCH<<"][2]["<<Nsamples<<"]/S";  tree->Branch("AnalogProbe",      analogProbe     .origin(), ss.str().c_str());
	  ss.str(""); ss<<"DigitalProbe["<<NCH<<"][4]["<<Nsamples<<"]/b"; tree->Branch("DigitalProbe",     digitalProbe    .origin(), ss.str().c_str());
	  ss.str(""); ss<<"AnalogProbeInfo[" <<NCH<<"][2]/b";             tree->Branch("AnalogProbeInfo",  analogProbeInfo .origin(), ss.str().c_str());
	  ss.str(""); ss<<"DigitalProbeInfo["<<NCH<<"][4]/b";             tree->Branch("DigitalProbeInfo", digitalProbeInfo.origin(), ss.str().c_str());
	  ss.str(""); ss<<"energy["<<NCH<<"]/s";                          tree->Branch("energy",           energy          .origin(), ss.str().c_str());
	  ss.str(""); ss<<"AnalogMultiFac[" <<NCH<<"][2]/b";              tree->Branch("AnalogMultiFac",   analogMultiFac  .origin(), ss.str().c_str());	  
	  fDoNotCutBranchYet = false;
	}
	//	cout<<"GetNumV2740Data() = "<<seg->GetNumV2740Data()<<endl;	      	    	
	for(int ifdata=0;ifdata<seg->GetNumV2740Data();ifdata++){
	  TArtRawV2740DataObject *fdata = seg->GetV2740Data(ifdata);
	  int fgeo = fdata->GetGeo();
	  int fch  = fdata->GetCh();

	  tsFADC                    = fdata->GetTimeStamp();
	  energy[fch]               = fdata->GetEnergy();

	  if(fdata->GetFlagIsLast()){
	    fFill = false;
	    cout<<" GetFlagIsLast() ="<<(int)fdata->GetFlagIsLast()<<endl;	    
	  }
	  else{
	    analogProbeInfo[fch][0]   = fdata->GetAnalogProbeInfo(0);
	    analogProbeInfo[fch][1]   = fdata->GetAnalogProbeInfo(1);
	    digitalProbeInfo[fch][0]  = fdata->GetDigitalProbeInfo(0);
	    digitalProbeInfo[fch][1]  = fdata->GetDigitalProbeInfo(1);
	    digitalProbeInfo[fch][2]  = fdata->GetDigitalProbeInfo(2);
	    digitalProbeInfo[fch][3]  = fdata->GetDigitalProbeInfo(3);


	    for(int iaprobe=0;iaprobe<2;iaprobe++){
	      uint8_t temp = (analogProbeInfo[fch][iaprobe]&0b00110000)>>4;
	      //	    uint8_t temp = analogProbeInfo[fch][iaprobe];
	      //	    cout<<"MultiFac["<<iaprobe<<"] = "<<(int)analogProbeInfo[fch][iaprobe]<<" ";
	      //	    cout<<"MultiFac["<<iaprobe<<"] = "<<(int)(analogProbeInfo[fch][iaprobe]&0b00110000)<<" ";	    
	      //	    cout<<"MultiFac["<<iaprobe<<"] = "<<(int)((analogProbeInfo[fch][iaprobe]&0b00110000)>>4)<<" ";
	      switch(temp){
	      case 0:
		analogMultiFac[fch][iaprobe] = 1;
		break;
	      case 1:
		analogMultiFac[fch][iaprobe] = 4;
		break;	      
	      case 2:
		analogMultiFac[fch][iaprobe] = 8;
		break;
	      case 3:
		analogMultiFac[fch][iaprobe] = 16;
		break;
	      default:
		analogMultiFac[fch][iaprobe] = 0;
		break;
	      }
	    }
	    //	  cout<<endl;

	    //	  cout<<"fch = "<<fch<<" TimeStamp = "<<tsFADC<<endl;
	    //	  cout<<"TimeStamp2 = "<<tsFADC<<" energy2 = "<<fdata->GetEnergy()<<endl;	      	    	  	  	  

	    for(int itb=0;itb<Nsamples;itb++){
	      if(!ifdata){
		timeBacket[itb] = fdata->GetTimeBacket(itb);
	      }

	      fadc[fch][itb] = fdata->GetVal(itb);
	      analogProbe[fch][0][itb] = fdata->GetAnalogProbe(0, itb);
	      analogProbe[fch][1][itb] = fdata->GetAnalogProbe(1, itb);
	      digitalProbe[fch][0][itb] = fdata->GetDigitalProbe(0, itb);
	      digitalProbe[fch][1][itb] = fdata->GetDigitalProbe(1, itb);
	      digitalProbe[fch][2][itb] = fdata->GetDigitalProbe(2, itb);
	      digitalProbe[fch][3][itb] = fdata->GetDigitalProbe(3, itb);

	      //	    
	      //	    cout<<"TimeBacket = "<<itb
	      //		<<" AnalogProbe0 = "<<analogProbe[2*fch+0][itb]
	      //		<<" AnalogProbe1 = "<<analogProbe[2*fch+1][itb]
	      //		<<" DigitalProbe0 = "<<digitalProbe[4*fch+0][itb]
	      //		<<" DigitalProbe1 = "<<digitalProbe[4*fch+1][itb]
	      //		<<" DigitalProbe2 = "<<digitalProbe[4*fch+2][itb] 
	      //		<<" DigitalProbe3 = "<<digitalProbe[4*fch+3][itb]
	      //		<<endl;
	    }
	  }
	}
      }
    }
      
    if(fFill){
      tree->Fill();
      rawEvent->Clear();
      estore->ClearData();
      fill_n(timeBacket      .data(), timeBacket      .num_elements(), 0);
      fill_n(fadc            .data(), fadc            .num_elements(), 0);
      fill_n(analogProbe     .data(), analogProbe     .num_elements(), 0);
      fill_n(digitalProbe    .data(), digitalProbe    .num_elements(), 0);
      fill_n(analogProbeInfo .data(), analogProbeInfo .num_elements(), 0);
      fill_n(digitalProbeInfo.data(), digitalProbeInfo.num_elements(), 0);
      fill_n(energy          .data(), energy          .num_elements(), 0);                              
    }
    
    //    if(!(counter%100)){
    cout<<"\rcounter = "<<counter;
      //    }
    counter++;
  }
  cout<<endl;  
  fout->cd();
  tree->Write();
  fout->Close();

  return 0;
}




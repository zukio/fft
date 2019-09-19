//線にグロウをかける
//サークルを動かす
//ずっと背景白だと線が見えなくなる
//前の音との高低差が大きいとき紙吹雪

boolean status;

int beatNum, typeNum;
float eRadius;
float eZoom;
float eAlpha;
float volume;
float max;
float velocity;
boolean hide = false;


color bg;
color strokeColor;

color colKick;
color colSnare;
color colHat;


void setup(){
  size(1280, 800, P3D);
  //fullScreen(P3D);
  smooth();
  colorMode(HSB, 360, 100, 100, 100);
  bg = color(0, 0, 0);
  strokeColor = color(0, 0, 100);
}

void musicSetup(){
  colKick = color(330, 100, 100);
  colSnare = color(220, 100, 100);
  colHat = color(280, 100, 100);
  
  //音の設定はまとめてminimタブに書いたよ
  setupMusic();
    
  //背景グリッド
  grid();
  
  ellipseMode(RADIUS);
  eRadius = 20;
  eZoom = 0;
  max = height*0.7;
}

void draw(){
  background(bg);
  translate(width/2, height/2);  
  if(!status){
    drawStart();
  } else {
    drawMusic();
  }
}


void drawMusic(){

  
  //音の読み込みはまとめてminimタブに書いたよ
  updateMusic();
    
  //ビートを解析
  beatCnt();
  
  
  noFill();
  //円周上にビートの分だけ点を打つ
  if(typeNum > 1){
    //種類の分だけ増やす
    polyPos(volume, beatNum, typeNum);
  } else {
    pop = new Drop(volume, beatNum);
    pop.drops(strokeColor);
  }
  
  //レンジを解析
  colorChange();
  
}

void colorChange(){
  //int lowBand = 5;
  //int highBand = 15;
  //int numberOfOnsetsThreshold = 4;
  // 4つ続けてlowとhighの範囲内なら色を変える
  //if ( beatFREQ.isRange(lowBand, highBand, numberOfOnsetsThreshold) ){
  //  hide = !hide;
  //}
  if(hide == true){
    if(brightness(bg) == 0) bg = color(0, 0, 100);
    bg = color(0, 0, constrain(brightness(bg) * 0.9999, 7, 100));
    strokeColor = color(0, 0, 0);
    blendMode(MULTIPLY);
    //blendMode(BLEND);
  } else {
    bg = color(0, 0, 0);
    strokeColor = color(0, 0, 100);
    //blendMode(LIGHTEST);
    blendMode(ADD);
  }
}


void beatCnt(){
  beatNum = typeNum = 0;
  //detectSize()を使うと、周波数モードでは現在使用されている周波数帯域の数を（サウンドモードでは常に0を）返します。
  for(int i = 0; i < beatFREQ.detectSize(); ++i){
    //isOnset(i)関数を使用すると、周波数モードではビートが検出された時にtrueを返します
    if ( beatFREQ.isOnset(i) ){ beatNum ++; }
   }
  if ( beatFREQ.isKick() ){typeNum ++; }
  if ( beatFREQ.isSnare() ){ typeNum ++; }
  if ( beatFREQ.isHat() ){ typeNum ++; }

  //isOnset()関数を使用すると、サウンドモードではビートが検出された時にtrueを返します
  if ( beatSOUND.isOnset() ){
    eRadius = typeNum*gridSize/2;
    eAlpha = 50;
    eZoom = 1;
  }
  eRadius = constrain(eRadius*0.95, 3, typeNum*gridSize/2);
  eAlpha *= 0.95; 
  eZoom -= 0.001*eAlpha;
  
  //周波数を解析
  gridPict();
}
    color col;

void gridPict(){
  pushMatrix();
  //目立つメロディラインを書き出す
  translate(-width/2, 0);
  float drawfft;
  float s;
  noStroke();
  if(fft.getBand(73) > fft.getBand(173)){
    fill(colKick);
    drawfft = fft.getBand(73);
  } else {
    fill(colHat);
    drawfft = fft.getBand(173);
  }
  for(int i = 0; i < initBand; i++){
    s = random(6);
    ellipse(map(i, 0, initBand, 0, width), sin(drawfft*i)*volume/2, s*eZoom, s*eZoom);
  }
  translate(width/2, 0);
  //背景グリッドに周波数域サークルを描画します
  //for(int i = 0; i <  grid; i++){
  for(int i = 0; i <  fft.avgSize(); i++){
    noStroke();
    boolean star = false;
    if(beatFREQ.isKick()) col = colKick; star = true;
    if(beatFREQ.isSnare()) col = colSnare; star = true;
    if(beatFREQ.isHat()) col = colHat; star = true;
    if(i == 0){
      stroke(255);
      //波形を描く
      int TimingColorChange = 0;
      for( int j = 0; j < song.bufferSize() - 1; j++ ){
        // 波形を線で描く場合、横位置（X座標）を求める
        float x1  =  map( j, 0, song.bufferSize(), -width/2, width/2 );
        float x2  =  map( j+1, 0, song.bufferSize(), -width/2, width/2 );
        // 縦位置（Y座標） from one buffer position to the next for both channels
        //line( x1, - song.left.get(j)*50,  x2, - song.right.get(j+1)*50);
        line( x1, - song.mix.get(j)*50,  x2, - song.mix.get(j+1)*50);
        
        // 波形を円で描く場合、1点ごとに進む角度（角速度）を求める
        velocity  =  map( j, 0, song.bufferSize(), 0, 360 );
        line(gridPos[i].x, gridPos[i].y, song.mix.get(j)*fft.getAvg(i) * sin(radians(velocity)),  song.mix.get(j+1)*fft.getAvg(i) *cos(radians(velocity)));
        
        // ループのついでに、背景を変えるフラグを立てる
        if(abs(song.mix.get(j)) > 0.999){
          TimingColorChange++;
        }
      } 
      //背景を変えるフラグを数える(激しい曲のときは数字を増やす)
      if(TimingColorChange > 8) hide = !hide;
    } else {
      //ズーム
      translate(0, 0, eZoom);
      //ダイヤ
      fill(col, eAlpha);
      if(star) astStar(gridPos[i].x, gridPos[i].y, fft.getAvg(i));
      //サークル
      fill(strokeColor, eAlpha);
      ellipse(gridPos[i].x, gridPos[i].y, fft.getAvg(i), fft.getAvg(i));
    }
    stroke(strokeColor, eAlpha);
  }
  popMatrix();
}
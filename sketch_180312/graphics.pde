Drop pop;


void polyPos(float R, int vertexs, int typeNum){
  for (int p = 0; p < typeNum; p++) {
    pushMatrix();
    translate((R)*cos(radians(360*p/typeNum)), (R)*sin(radians(360*p/typeNum)));
    rotate(radians(360*p/typeNum));
    pop = new Drop(R, vertexs);
    pop.drops(strokeColor);
    popMatrix();
  }
}

void polygon(float R, int vertexs){
  rotate(velocity*frameCount);
  beginShape();
  for (int i = 0; i < vertexs; i++) {
    vertex(R*cos(radians(360*i/vertexs)), R*sin(radians(360*i/vertexs)));
  }
  endShape(CLOSE);
}

void astStar(float x, float y, float R){
  pushMatrix();
  translate(x, y);
  beginShape();
  float tateYokoHi = 1;
  for (int theta = 0; theta < 360; theta++) {
    vertex(R * pow(cos(radians(theta)), 3), R * tateYokoHi * pow(sin(radians(theta)), 3));
  }
  endShape(CLOSE);
  popMatrix();  
}

class Drop{
  private int vertexs;  
  private float R;  
  float dropsR;
  float dropsAlpha;
  float ownRa;
  
  Drop(float R, int vertexs){
    this.vertexs = vertexs;
    this.R = R;
    dropsR = 1;
    dropsAlpha = 100;
    ownRa = 0.1;
  }
  
  void drops(color col){
    while(dropsR < R){
      stroke(col, dropsAlpha);
      for (int i = 0; i < vertexs; i++) {
        //ellipse(dropsR*cos(radians(360*i/vertexs)), dropsR*sin(radians(360*i/vertexs)), eRadius*ownRa, eRadius*ownRa);
        pushMatrix();
        translate(dropsR*cos(radians(360*i/vertexs)), dropsR*sin(radians(360*i/vertexs)));
        rotate(radians(360*i/vertexs));
        polygon(eRadius*ownRa, vertexs);
        popMatrix();
      }
      dropsR *= 1.5;
      dropsAlpha *= 0.9;
      ownRa *= 1.5; 
    }
  }
}


PVector[] gridPos;
int grid, gridH, gridW;
float gridSize;
void grid(){
  fill(strokeColor);
  grid = beatFREQ.detectSize();
  grid *= 4; //(27*4=108)
  gridPos = new PVector[grid];
  ///縦全部で何個、横全部で何個（モニタの比率16：9を使用）
  //gridH = round(9*grid/(16*9));
  //gridW = round(grid/gridH);
  ///縦と横でマスの数を同じにする場合
  gridH = int(sqrt(grid));
  gridW = gridH;
  //1マスのサイズ
  float xWidth = width/gridW;
  float yHeight = height/gridH;
  //グリッドの配置に必要な変数
  int turnPoint = 1;
  int steps = 0;
  int passPointCnt = 0;
  int direction = 0;
  //座標
  float x = 0;
  float y = 0;
  for(int i = 0; i < grid; i++){
    //折り返し位置まで進める
    if(steps <= turnPoint && i != 0){
      //まず右へ
      if(direction == 0) x += xWidth;
      //下へ
      if(direction == 1) y += yHeight;
      //左へ
      if(direction == 2) x -= xWidth;
      //上へ
      if(direction == 3) y -= yHeight;
    }
    
    //折り返し回数ぶん折り返したら折り返し位置を増やす
    if(passPointCnt == 2){
      //折り返し位置を増やす
      turnPoint += 1;
      //折り返し回数をカウント
      passPointCnt = 0;
    }
    
    //折り返し位置まできたら折り返す
    if(steps == turnPoint){
      //折り返し回数をカウント
      passPointCnt++;
      //方向を変える
      direction += 1;
      //カウントをリセット
      steps = 0;
      if(direction == 4) direction = 0;
    }
    
    //カウント
    steps++;
   
    gridPos[i] = new PVector(x, y);
  }
}
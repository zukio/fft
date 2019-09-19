int rectWidth = 150;
int rectHight = 50;
String songName; 

void drawStart(){
  float posXc = 16 + rectWidth/2;
  color colL = color(0);
  color colR = color(0);
  
  if((mouseY < height/2+rectHight/2) && (mouseY > height/2-rectHight/2)){
    if((mouseX < width/2-8) && (mouseX > width/2-rectWidth-8)){
      colL = color(100);
    } else if((mouseX > width/2+8) && (mouseX < width/2+rectWidth+8)){
      colR = color(100);
   }
  }
  stroke(strokeColor);
  rectMode(CENTER);
  fill(colL);
  rect(-posXc, 0, rectWidth, rectHight);
  fill(colR);
  rect(posXc, 0, rectWidth, rectHight);
  
  fill(255);
  noStroke();
  textAlign(CENTER);
  text("Charisma.com", -posXc, 5);
  text("AI", posXc, 5);
  
}

void mouseClicked(){
  if(!status){
    boolean button = false;
    if((mouseY < height/2+rectHight/2) && (mouseY > height/2-rectHight/2)){
      if((mouseX < width/2-8) && (mouseX > width/2-rectWidth-8)){
        songName = "marcus_kellis_theme.mp3";
        button = true;
      } else if((mouseX > width/2+8) && (mouseX < width/2+rectWidth+8)){
        songName = "AUmarcus_kellis_theme.mp3";
        button = true;
      }
    }
    if(button){
      musicSetup();
      status = true;
    }
  } else {
    status = !status;
    song.close();
  }
}
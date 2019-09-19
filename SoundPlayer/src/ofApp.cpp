#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    smoothing = 0.5;
    setupAnalyzer();
    setupVisual();
}
//サウンド解析用
void ofApp::setupAnalyzer(){
    sampleRate = 44100;
    bufferSize = 512; //256?
    int channels = 2; //1?
    
    audioAnalyzer.setup(sampleRate, bufferSize, channels);
    
    //サウンドファイルの読み込み（ファイル名、ストリーミング）
    player.load("jingle.mp3", true);
    nowOnPlay = false;

}
//ビジュアル描画用
void ofApp::setupVisual(){
    //色の設定
    ofColor::fromHsb(360, 100, 100);
    ofBackground(0, 0, 0);
    ofEnableSmoothing();
    //円を何角形で描画するか
    ofSetCircleResolution(64);
    //混色を加算に
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    //画像の読み込み
    baseImg.load("tonemappet.jpg");
    reScale = ofGetWidth()/baseImg.getWidth();
    //画像をピクセル解析して座標データに変換
    getImgPos();
    //演出用
    spark = 150;
    
    //座標確認用
    position.x = 0;
    position.y = 0;
    dataView = false;

}
//--------------------------------------------------------------
void ofApp::update(){
    updateVisual();
    updateAnalyzer();
}
//サウンド解析用
void ofApp::updateAnalyzer(){
    //-:Get buffer from sound player:
    soundBuffer = player.getCurrentSoundBuffer(bufferSize);
    //-:ANALYZE SOUNDBUFFER:
    audioAnalyzer.analyze(soundBuffer);
    //-:get Values:
    getAllValues();
}
//ビジュアル描画用
void ofApp::updateVisual(){
    //周波数（waveform for channel）
    spectrum_l = audioAnalyzer.getValues(SPECTRUM, 0);
    spectrum_r = audioAnalyzer.getValues(SPECTRUM, 1);
    //周波数ポリゴン描画用（Waveform update:)
    waveform_l.clear();
    waveform_r.clear();
    updateWaveForms(ofGetWidth()/2, ofGetHeight()/2);
}


//--------------------------------------------------------------
void ofApp::draw(){
    if(nowOnPlay){
        drawAnalyzer();
        if(dataView){
            drawDataView();
        } else {
            drawVisual();
            //drawWaveForms();
        }
    } else {
        int w = 10;
        int h = 30;
        ofSetColor(255);
        ofRectMode(OF_RECTMODE_CENTER);
        ofDrawRectangle(ofGetWidth()/2 - w, ofGetHeight()/2, w, h);
        ofDrawRectangle(ofGetWidth()/2 + w, ofGetHeight()/2, w, h);
    }
}
//サウンド解析用
void ofApp::drawAnalyzer(){
}
//ビジュアル描画用
void ofApp::drawVisual(){
    baseImg.draw(0, 0, ofGetWidth(), baseImg.getHeight()*reScale);
    //画像に重ねて図形を描画
    //waveform for channel 0
    //spectrum_l;
    int ch=0;
    for(size_t i = 0; i < soundBuffer.getNumFrames(); i++) {
        float sample = soundBuffer.getSample(i, ch);
        float x = ofMap(i, 0, soundBuffer.getNumFrames(), 0, ofGetWidth());
        float y = ofMap(sample, -1, 1, 0, 50);
        ofDrawCircle(x, y, pow(power, 2)*ofNoise(1.0));
    }
    //melBands[m]*hpcp[h]power*
    for(int i=0; i < drawPoint.size(); i++){
        //描画座標を登録した配列の順番を、シャッフルした順番に入れ替え
        int shuffled_i = shuffledNum[i];
        //描画点数とサウンドサンプルの数を揃える
        int n = (int) ofMap(shuffled_i, 0, drawPoint.size(), 0, soundBuffer.getNumFrames());
        //描画
        ofSetColor(colors[shuffled_i], 200);
        ofDrawCircle(drawPoint[i].x*reScale,
                     drawPoint[i].y*reScale,
                     pow(5*soundBuffer.getSample(n,0), 3));
    }
    //別保存された特定の範囲を描く
    for(int i = 0; i < specialPoint.size(); i++){
        ofColor::fromHsb(195, 255, 255, 30);
        ofDrawCircle(specialPoint[i].x*reScale,
                     specialPoint[i].y*reScale,
                     5*ofNoise(1.0)*isOnset);
    }
    //座標確認用
    ofSetColor(255);
    ofDrawBitmapString(position.x*1/reScale, position.x, position.y-10);
    ofDrawBitmapString(position.y*1/reScale, position.x, position.y);
    
}
//変数確認用
void ofApp::drawDataView(){
    
    //-Single value Algorithms:
    
    ofPushMatrix();
    ofTranslate(350, 0);
    int mw = 250;
    int xpos = 0;
    int ypos = 30;
    
    float value, valueNorm;
    
    ofSetColor(255);
    value = rms;
    string strValue = "RMS: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = power;
    strValue = "Power: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = pitchFreq;
    valueNorm = pitchFreqNorm;
    strValue = "Pitch Frequency: " + ofToString(value, 2) + " hz.";
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = pitchConf;
    strValue = "Pitch Confidence: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = pitchSalience;
    strValue = "Pitch Salience: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = inharmonicity;
    strValue = "Inharmonicity: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = hfc;
    valueNorm = hfcNorm;
    strValue = "HFC: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = specComp;
    valueNorm = specCompNorm;
    strValue = "Spectral Complexity: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = centroid;
    valueNorm = centroidNorm;
    strValue = "Centroid: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = dissonance;
    strValue = "Dissonance: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = rollOff;
    valueNorm = rollOffNorm;
    strValue = "Roll Off: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw , 10);
    
    ypos += 50;
    ofSetColor(255);
    value = oddToEven;
    valueNorm = oddToEvenNorm;
    strValue = "Odd To Even Harmonic Energy Ratio: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = strongPeak;
    valueNorm = strongPeakNorm;
    strValue = "Strong Peak: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = strongDecay;
    valueNorm = strongDecayNorm;
    strValue = "Strong Decay: " + ofToString(value, 2);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, valueNorm * mw, 10);
    
    ypos += 50;
    ofSetColor(255);
    value = isOnset;
    strValue = "Onsets: " + ofToString(value);
    ofDrawBitmapString(strValue, xpos, ypos);
    ofSetColor(ofColor::cyan);
    ofDrawRectangle(xpos, ypos+5, value * mw, 10);
    
    ofPopMatrix();
    
    //-Vector Values Algorithms:
    
    ofPushMatrix();
    
    ofTranslate(700, 0);
    
    int graphH = 75;
    int yoffset = graphH + 50;
    ypos = 30;
    
    ofSetColor(255);
    ofDrawBitmapString("Spectrum: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    float bin_w = (float) mw / spectrum.size();
    for (int i = 0; i < spectrum.size(); i++){
        float scaledValue = ofMap(spectrum[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    ypos += yoffset;
    ofSetColor(255);
    ofDrawBitmapString("Mel Bands: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / melBands.size();
    for (int i = 0; i < melBands.size(); i++){
        float scaledValue = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    ypos += yoffset;
    ofSetColor(255);
    ofDrawBitmapString("MFCC: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / mfcc.size();
    for (int i = 0; i < mfcc.size(); i++){
        float scaledValue = ofMap(mfcc[i], 0, MFCC_MAX_ESTIMATED_VALUE, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    ypos += yoffset;
    ofSetColor(255);
    ofDrawBitmapString("HPCP: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / hpcp.size();
    for (int i = 0; i < hpcp.size(); i++){
        //float scaledValue = ofMap(hpcp[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float scaledValue = hpcp[i];
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
    
    ypos += yoffset;
    ofSetColor(255);
    ofDrawBitmapString("Tristimulus: ", 0, ypos);
    ofPushMatrix();
    ofTranslate(0, ypos);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / tristimulus.size();
    for (int i = 0; i < tristimulus.size(); i++){
        //float scaledValue = ofMap(hpcp[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float scaledValue = tristimulus[i];
        float bin_h = -1 * (scaledValue * graphH);
        ofDrawRectangle(i*bin_w, graphH, bin_w, bin_h);
    }
    ofPopMatrix();
}
//--------------------------------------------------------------
void ofApp::getImgPos(){
    //画像データのビットマップ情報を配列に格納
    ofPixels pixels = baseImg.getPixels();
    int w = baseImg.getWidth();
    int h = baseImg.getHeight();
    //画像をgridsピクセル感覚でスキャン(ベストは8)
    int grids = 2;
    for(int j=0; j < h; j+=grids){
        for(int i=0; i < w; i+=grids){
            //RGBを取得
            int valueR = pixels[j*3*w + i*3+3];
            int valueG = pixels[j*3*w + i*3+2];
            int valueB = pixels[j*3*w + i*3+1];
            int valueA = pixels[j*3*w + i*3]; //グレースケール
            ofColor colRgb = ofColor(valueR, valueG, valueB);
            //HSBに変換 ----------------------------
            ofColor colHsb = colRgb;
            //HSBの各要素ごとに取得
            float hue =0;
            float saturation = 0;
            float brightness = 0;
            colHsb.getHsb(hue, saturation, brightness);
            //座標を配列に格納 -----------------------
            if((valueA > 240 || brightness > 240)){
                //特定の範囲を除く
                //if(!(j < 400)){
                //if(!(j > 630 && i < 850)){
                if(!(j > 620 || j < 185 || (i > 930 && j < 206) || (i > 1000 && j < 226))){
                    ofVec2f pos= ofVec2f(i, j);
                    drawPoint.emplace_back(pos);
                    colors.emplace_back(colRgb);
                }
            }
            //特定の範囲を別に保存
            if(hue > 150 && hue < 190 && brightness > 150 && j < 300){
                ofVec2f pos= ofVec2f(i, j);
                specialPoint.emplace_back(pos);
            }
        }
    }
    shuffleID(drawPoint.size());
}
//--------------------------------------------------------------
void ofApp::shuffleID(int ArraySize){
    //シャッフルした番号リストを作成
    shuffledNum = vector<int>(ArraySize);
    //（1）まず番号を登録 for(int i=0; i < array.size(); i++){ shuffledNum[i] = i; } と同義
    iota(shuffledNum.begin(), shuffledNum.end(), 0);
    //（2）シャッフル
    random_shuffle(shuffledNum.begin(), shuffledNum.end());
}

//--------------------------------------------------------------
void ofApp::getAllValues(){
    rms     = audioAnalyzer.getValue(RMS, 0, smoothing);
    power   = audioAnalyzer.getValue(POWER, 0, smoothing);
    pitchFreq = audioAnalyzer.getValue(PITCH_FREQ, 0, smoothing);
    pitchConf = audioAnalyzer.getValue(PITCH_CONFIDENCE, 0, smoothing);
    pitchSalience  = audioAnalyzer.getValue(PITCH_SALIENCE, 0, smoothing);
    inharmonicity   = audioAnalyzer.getValue(INHARMONICITY, 0, smoothing);
    hfc = audioAnalyzer.getValue(HFC, 0, smoothing);
    specComp = audioAnalyzer.getValue(SPECTRAL_COMPLEXITY, 0, smoothing);
    centroid = audioAnalyzer.getValue(CENTROID, 0, smoothing);
    rollOff = audioAnalyzer.getValue(ROLL_OFF, 0, smoothing);
    oddToEven = audioAnalyzer.getValue(ODD_TO_EVEN, 0, smoothing);
    strongPeak = audioAnalyzer.getValue(STRONG_PEAK, 0, smoothing);
    strongDecay = audioAnalyzer.getValue(STRONG_DECAY, 0, smoothing);
    //Normalized values for graphic meters:
    pitchFreqNorm   = audioAnalyzer.getValue(PITCH_FREQ, 0, smoothing, TRUE);
    hfcNorm     = audioAnalyzer.getValue(HFC, 0, smoothing, TRUE);
    specCompNorm = audioAnalyzer.getValue(SPECTRAL_COMPLEXITY, 0, smoothing, TRUE);
    centroidNorm = audioAnalyzer.getValue(CENTROID, 0, smoothing, TRUE);
    rollOffNorm  = audioAnalyzer.getValue(ROLL_OFF, 0, smoothing, TRUE);
    oddToEvenNorm   = audioAnalyzer.getValue(ODD_TO_EVEN, 0, smoothing, TRUE);
    strongPeakNorm  = audioAnalyzer.getValue(STRONG_PEAK, 0, smoothing, TRUE);
    strongDecayNorm = audioAnalyzer.getValue(STRONG_DECAY, 0, smoothing, TRUE);
    
    dissonance = audioAnalyzer.getValue(DISSONANCE, 0, smoothing);
    
    spectrum = audioAnalyzer.getValues(SPECTRUM, 0, smoothing);
    melBands = audioAnalyzer.getValues(MEL_BANDS, 0, smoothing);
    mfcc = audioAnalyzer.getValues(MFCC, 0, smoothing);
    hpcp = audioAnalyzer.getValues(HPCP, 0, smoothing);
    
    tristimulus = audioAnalyzer.getValues(TRISTIMULUS, 0, smoothing);
    
    isOnset = audioAnalyzer.getOnsetValue(0);
    
}
//--------------------------------------------------------------
void ofApp::updateWaveForms(float widthLength, float maxHeight){
    //waveform for channel 0
    int ch=0;
    for(size_t i = 0; i < soundBuffer.getNumFrames(); i++) {
        float sample = soundBuffer.getSample(i, ch);
        float x = ofMap(i, 0, soundBuffer.getNumFrames(), 0, widthLength);
        float y = ofMap(sample, -1, 1, 0, maxHeight);
        waveform_l.addVertex(x, y);
    }
    
    //waveform for channel 1
    ch = 1;
    for(size_t i = 0; i < soundBuffer.getNumFrames(); i++) {
        float sample = soundBuffer.getSample(i, ch);
        float x = ofMap(i, 0, soundBuffer.getNumFrames(), 0, ofGetWidth()*0.5);
        float y = ofMap(sample, -1, 1, 0, ofGetHeight()*0.5) + ofGetHeight()*0.5;
        waveform_r.addVertex(x, y);
    }
}
void ofApp::drawWaveForms(){
    ofSetColor(ofColor::hotPink);
    waveform_l.draw();
    waveform_r.draw();
    
    //draw spectrums:
    int mw = ofGetWidth()/2;
    
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2, 0);
    ofSetColor(ofColor::cyan);
    float bin_w = (float) mw / spectrum_l.size();
    for (int i = 0; i < spectrum_l.size(); i++){
        float scaledValue = ofMap(spectrum_l[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * ofGetHeight()/2);
        ofDrawRectangle(i*bin_w, ofGetHeight()/2, bin_w, bin_h);
    }
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofSetColor(ofColor::cyan);
    bin_w = (float) mw / spectrum_r.size();
    for (int i = 0; i < spectrum_r.size(); i++){
        float scaledValue = ofMap(spectrum_r[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
        float bin_h = -1 * (scaledValue * ofGetHeight()/2);
        ofDrawRectangle(i*bin_w, ofGetHeight()/2, bin_w, bin_h);
    }
    ofPopMatrix();
    
    
    ofSetColor(100);
    ofDrawLine(ofGetWidth()/2, 0, ofGetWidth()/2, ofGetHeight());
    ofDrawLine(0, ofGetHeight()/2, ofGetWidth(), ofGetHeight()/2);
    ofSetColor(255);
    ofDrawBitmapString("ofxAudioAnalyzer - AUDIO FILE PLAYER EXAMPLE \nPress any key to play audio file ", 350, 32);
    
}

//--------------------------------------------------------------
void ofApp::exit(){
    audioAnalyzer.exit();
    player.stop();
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    dataView = !dataView;

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    //確認用
    position.x = x;
    position.y = y;

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    //クリックで再生
    nowOnPlay = !nowOnPlay;
    if(nowOnPlay){
        player.play();
    } else {
        player.stop();
    }

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

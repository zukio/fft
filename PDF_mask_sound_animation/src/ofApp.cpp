#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(120);
    smoothing = 0.5;
    //色の設定
    ofColor::fromHsb(360, 100, 100);
    ofEnableSmoothing();
    //円を何角形で描画するか
    ofSetCircleResolution(64);
    //混色を加算に
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    //演出用
    spark = 150;
    pose = false;
    dataView = false;
    reFrameCount = 0;
    frameCount = 0;

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
    player.load("wanima.mp3", true);
    nowOnPlay = false;
    
}
//ビジュアル描画用
void ofApp::setupVisual(){
    //PDF読み込み（アニメーションするならイラレデータはレイヤー分けしておいた方が扱いやすい）
    pdf_lipBase.loadPDF("lip_lip.pdf");//0グロス下, 1グロス上
    pdf_lipInner.loadPDF("lip_in.pdf"); //0口内, 1歯, 2舌
    pdf_lipMask.loadPDF("lip_inMask.pdf");//0唇(0-6内, 7-21外)
    //プレビュー画像のサイズを決定
    fbo_lipBase.allocate(ofGetWidth(), ofGetHeight());
    fbo_lipInner.allocate(ofGetWidth(), ofGetHeight());
    fbo_lipMask.allocate(ofGetWidth(), ofGetHeight());
    fbo_lipMaskC.allocate(ofGetWidth(), ofGetHeight());
    //プレビュー画像のアンカーポイント（重心軸）を決定
    fbo_lipBase.setAnchorPoint(pdf_lipBase.getWidth()/2, pdf_lipBase.getHeight()/2);
    fbo_lipInner.setAnchorPoint(pdf_lipInner.getWidth()/2, pdf_lipInner.getHeight()/2);
    fbo_lipMask.setAnchorPoint(pdf_lipMask.getWidth()/2, pdf_lipMask.getHeight()/2);
    //取得したデータはアニメーションするために書き換えるので元データを複製しておく
    original_lipBase.loadPDF("lip_lip.pdf");
    original_lipInner.loadPDF("lip_in.pdf");
    original_lipMask.loadPDF("lip_inMask.pdf");
    /*元データをまるまる複製するのではなく使用する情報だけ抜き取るパターン
    for (int i = 0; i < original_lipBase.getNumPath(); i++){
        ofPath path = original_lipBase.getPathAt(i);
        //アニメーション用に座標情報だけ別に保存しておきます
        vector<ofPoint> access_i;
        for(int k = 0; k < path.getCommands().size(); k++){
            ofPoint access_i_k = ofPoint(path.getCommands()[k].to.x, path.getCommands()[k].to.y);
            access_i.emplace_back(access_i_k);
        }
        access.emplace_back(access_i);
        access_i.erase(access_i.begin(), access_i.end());
        //アニメーション用にラインを別に保存しておきます
        path.setPolyWindingMode(OF_POLY_WINDING_ODD); // パスの中窓を抜くか合体するかなどのモード
        vector<ofPolyline>& lines = const_cast<vector<ofPolyline>&>(path.getOutline());
        for(int j=0;j<(int)lines.size();j++){
            outlines.emplace_back(lines[j].getResampledBySpacing(1));
        }
    }*/
}


//--------------------------------------------------------------
void ofApp::update(){

    updateVisual();
    updateAnalyzer();
    reFrameCount++;
    if(reFrameCount%4 == 0){
        float hfcForCount = ofClamp(hfc, 0, 512);
        frameCount = ofMap(hfcForCount, 0, 512, 90, 315);
    }
    animPitch = pitchConf;
    ReAnimPitch = 0;
    if(animPitch > 0.8){
        ReAnimPitch = ofClamp(animPitch-0.8, 0, 0.353);
        animPitch = 0.8;
    } else if(animPitch < 0.5){
        animPitch = 0.5;
    }
}
//サウンド解析用
void ofApp::updateAnalyzer(){
    //-:Get buffer from sound player:
    soundBuffer = player.getCurrentSoundBuffer(bufferSize);
    //-:ANALYZE SOUNDBUFFER:
    audioAnalyzer.analyze(soundBuffer);
    //-:get Values:
    getAllValues();
    //周波数（waveform for channel）
    spectrum_l = audioAnalyzer.getValues(SPECTRUM, 0);
    spectrum_r = audioAnalyzer.getValues(SPECTRUM, 1);
    //周波数ポリゴン描画用（Waveform update:)
    waveform_l.clear();
    waveform_r.clear();
    updateWaveForms(ofGetWidth()/2, ofGetHeight()/2);
}
//ビジュアル描画用
void ofApp::updateVisual(){
    ofBackground(0, 0, 0);
   //マスクをかけるPDFをプレビュー
    fbo_lipInner.begin();
    ofClear(0);
    lipInner(pdf_lipInner);
    fbo_lipInner.end();
    //マスクをプレビュー
    fbo_lipMask.begin();
    ofClear(0);
    lipMask(pdf_lipMask);
    fbo_lipMask.end();
    //マスクのアンカーをコーナーから中心に直して再描画
    fbo_lipMaskC.begin();
    ofClear(0);
    ofPushMatrix();
    ofTranslate(pdf_lipMask.getWidth()/2, pdf_lipMask.getHeight()/2);
    if(pose) ofScale(PingPongEase(animPitch, 0.8, frameCount).x, PingPongEase(0, 1+ReAnimPitch, frameCount).y);
    fbo_lipMask.draw(0, 0);
    ofPopMatrix();
    fbo_lipMaskC.end();
    //マスクをかける
    fbo_lipInner.getTexture().setAlphaMask(fbo_lipMaskC.getTexture());
    //背景画像のPDFをプレビュー
    fbo_lipBase.begin();
    ofClear(0);
    lipBase(pdf_lipBase);
    fbo_lipBase.end();
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
    //再生中の帯域の音量を取得
    float* val = ofSoundGetSpectrum(1);
    //現在の音量を取得
    volume = val[0];
}
//ビジュアル描画用
void ofApp::drawVisual(){
    ofPushMatrix();
        ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofScale(0.8, 0.8);
        //背景画像を描画する
        ofPushMatrix();
            if(pose) ofScale(PingPongEase(animPitch, 1+ReAnimPitch, frameCount).x, PingPongEase(0.353, 1, frameCount).y);
            fbo_lipBase.draw(0, 0);
        ofPopMatrix();
        //マスクをかけた画像を描画する
        fbo_lipInner.draw(0,0);
    ofPopMatrix();
    //確認用
//    ofPushMatrix();
//        ofTranslate(ofGetWidth()/2-pdf_lipBase.getWidth()/2, ofGetHeight()/2-pdf_lipBase.getHeight()/2);
//        for(int i = 0; i < pdf_lipBase.getNumPath(); i++){
//            ofPath& path = pdf_lipBase.getPathAt(i);
//            for(int k = 0; k < path.getCommands().size(); k++){
//                if(i == 0){
//                    ofPoint cp1 = path.getCommands()[k].cp1;
//                    ofPoint cp2 = path.getCommands()[k].cp2;
//                    ofPoint to = path.getCommands()[k].to;
//                    ofDrawLine(path.getCommands()[k-1].cp2, to);
//                    ofDrawLine(to, cp1);
//                    ofDrawLine(cp2, path.getCommands()[k+1].to);
//                    ofDrawCircle(to, 4);
//                    ofDrawCircle(cp1, 2);
//                    ofDrawCircle(cp2, 2);
//                    ofDrawBitmapString(ofToString(k), to);
//                    string note1 = "Base." +ofToString(i) +"-"+ ofToString(k) +"cp1";
//                    ofDrawBitmapString(note1, cp1);
//                    string note2 = "Base." + ofToString(i) +"-"+ ofToString(k) +"cp2";
//                    ofDrawBitmapString(note2, cp2);
//                }
//            }
//        }
//    for(int i = 0; i < pdf_lipInner.getNumPath(); i++){
//        for(int k = 0; k < pdf_lipInner.getCommands().size(); k++){
//            ofPoint cp1 = path.getCommands()[k].cp1;
//            ofPoint cp2 = path.getCommands()[k].cp2;
//            ofPoint to = path.getCommands()[k].to;
//    ofDrawLine(path.getCommands()[k-1].cp2, to);
//    ofDrawLine(to, cp1);
//    ofDrawLine(cp2, path.getCommands()[k+1].to);
//    ofDrawCircle(to, 5);
//            ofDrawCircle(cp1.x, cp1.y, 2);
//            ofDrawCircle(cp2.x, cp2.y, 2);
//            string note1 = ofToString(i) +"-"+ ofToString(k) +"cp1";
//            ofDrawBitmapString(note1, cp1.x, cp1.y);
//            string note2 = ofToString(i) +"-"+ ofToString(k) +"cp2";
//            ofDrawBitmapString(note2, cp2.x, cp2.y);
//        }
//    }
//    for(int i = 0; i < pdf_lipMask.getNumPath(); i++){
//        ofPath& path = pdf_lipMask.getPathAt(i);
//        for(int k = 0; k < path.getCommands().size(); k++){
//            ofPoint cp1 = path.getCommands()[k].cp1;
//            ofPoint cp2 = path.getCommands()[k].cp2;
//            ofPoint to = path.getCommands()[k].to;
//            ofDrawLine(path.getCommands()[k-1].cp2, to);
//            ofDrawLine(to, cp1);
//            ofDrawLine(cp2, path.getCommands()[k+1].to);
//            ofDrawCircle(to, 5);
//            ofDrawCircle(cp1.x, cp1.y, 2);
//            ofDrawCircle(cp2.x, cp2.y, 2);
//            ofDrawBitmapString(ofToString(k), to);
//            string note1 =  "mask." +ofToString(i) +"-"+ ofToString(k) +"cp1";
//            ofDrawBitmapString(note1, cp1.x, cp1.y);
//            string note2 =  "mask." +ofToString(i) +"-"+ ofToString(k) +"cp2";
//            ofDrawBitmapString(note2, cp2.x, cp2.y);
//        }
//    }
    ofPopMatrix();
}
//--------------------------------------------------------------
//PDF解析用
void ofApp::lipInner(ofxPDF pdfData){
    for(int i = 0; i < pdfData.getNumPath(); i++){
        //ひとつずつPDFデータを読み込みます
        ofPath& path = pdfData.getPathAt(i);
        ofPath& originalPath = original_lipInner.getPathAt(i);
        //例えば色データを読み込んでみるよ
        //(1)RGB
        ofColor rgb = ofColor((unsigned int)path.getFillColor().r,
                              (unsigned int)path.getFillColor().g,
                              (unsigned int)path.getFillColor().b);
        //(2)白黒情報
        float alpha = (unsigned int)path.getFillColor().a;
        //(3)HSBに変換
        ofColor hsb = rgb;
        float hue; float saturation; float brightness;
        hsb.getHsb(hue, saturation, brightness);
        //(4)B 特定の色だけ塗り替えてみよう
        if(saturation > 150){
            //(4)A 色をアニメーション
            hue = (int)round(rotateCount(0, 360, frameCount));
            if(saturation < 5) saturation = 5;
            if(brightness > 250) brightness = 250;
            hsb.setHsb(hue, saturation, brightness);
            //path.setFillColor(hsb);
        }
        //次は座標を読み込んでみるよ
        //cout << "Tess:" << &path.getTessellation() << endl;
        for(int k = 0; k < path.getCommands().size(); k++){
            //ベジェハンドル
            ofPoint cp1 = originalPath.getCommands()[k].cp1;
            ofPoint cp2 = originalPath.getCommands()[k].cp2;
            //コマンドタイプ
            typename ofPath::Command::Type type = originalPath.getCommands()[k].type;
            string typeName;
            if(type == 0) typeName = "moveTo";
            if(type == 1) typeName = "lineTo";
            if(type == 2) typeName = "curveTo";
            if(type == 3) typeName = "bezierTo";
            if(type == 4) typeName = "quadBezierTo";
            if(type == 5) typeName = "arc";
            if(type == 6) typeName = "?";
            if(type == 7) typeName = "close";
            //曲率
            float r1 = originalPath.getCommands()[k].radiusX;
            float r2 = originalPath.getCommands()[k].radiusY;
            //if(r1 > 0.1) cout << "R1:" << r1 << endl;
            //if(r2 > 0.1) cout << "R2:" << r2 << endl;
            //座標
            ofPoint to = originalPath.getCommands()[k].to;
        }
        //描画
        path.draw();
    }
}
void ofApp::lipBase(ofxPDF pdfData){
    for(int i = 0; i < pdfData.getNumPath(); i++){
        //ひとつずつPDFデータを読み込みます
        ofPath& path = pdfData.getPathAt(i);
        ofPath& originalPath = original_lipBase.getPathAt(i);
        for(int k = 0; k < path.getCommands().size(); k++){
            //ベジェハンドル
            ofPoint cp1 = originalPath.getCommands()[k].cp1;
            ofPoint cp2 = originalPath.getCommands()[k].cp2;
            //座標
            ofPoint to = originalPath.getCommands()[k].to;
            //アニメーションでポイントを動かす（上中央を1として左回りに8(0)までナンバリング）
            ofVec2f move = PingPongEase(0, 90, frameCount);
            ofVec2f move2 = plusMinusEase(180, frameCount/2);
            if(i == 0 && pose){
                //口角を上げる（0-3が左端、0-7が右端）
                if(k == 3){
                    path.getCommands()[k].to.y = to.y - move.x;
                    path.getCommands()[k].cp2.y = cp2.y + move.x;
                }
                if(k == 7){
                    path.getCommands()[k].to.y = to.y - move.x;
                    path.getCommands()[k].cp2.y = cp2.y + move.x;
                }
                /*左右に歪ませる（1,2,8(0)が上、4-6が下）
                if(k <= 2 || k == 3 || k == 8){
                    if(k != 3){
                        path.getCommands()[k].to.x = to.x + move2.x*2;
                        path.getCommands()[k].cp2.x = cp2.x + move2.x*2;
                    }
                    if(k != 8)path.getCommands()[k].cp1.x = cp1.x + move2.x*2;
                }
                if((k <= 6 && k >= 4) || k == 7){
                    if(k != 7){
                        path.getCommands()[k].to.x = to.x - move2.x*2;
                        path.getCommands()[k].cp2.x = cp2.x - move2.x*2;
                        //下に
                        //下にpath.getCommands()[k].to.y = to.y + move.y/2;
                        //下にpath.getCommands()[k].cp2.y = cp2.y + move.y/2;
                    }
                    if(k != 4)path.getCommands()[k].cp1.x = cp1.x - move2.x*2;
                    //下にpath.getCommands()[k].cp1.y = cp1.y + move.y/2;
                 }*/
            }
            //上のグロスはi == 2, 下はi == 1
        }
        //描画
        path.draw();
    }
}
void ofApp::lipMask(ofxPDF pdfData){
    for(int i = 0; i < pdfData.getNumPath(); i++){
        //ひとつずつPDFデータを読み込みます
        ofPath& path = pdfData.getPathAt(i);
        ofPath& originalPath = original_lipMask.getPathAt(i);
        //path.setMode(ofPath::POLYLINES);
//        ofPolyline & line = outlines[i];
//        ofBeginShape();
//        for (int j = 0; j < line.size(); j++){
//            ofVertex(line[j]);
//        }
//        ofEndShape();
        for(int k = 0; k < path.getCommands().size(); k++){
            //ベジェハンドル
            ofPoint cp1 = originalPath.getCommands()[k].cp1;
            ofPoint cp2 = originalPath.getCommands()[k].cp2;
            //座標
            ofPoint to = originalPath.getCommands()[k].to;
            //アニメーション（下中央を0として左回りに6(0)までナンバリング）
            if(i == 0 && pose){
                ofVec2f move = PingPongEase(0, 90, frameCount);
                ofVec2f move2 = plusMinusEase(180, frameCount/2);
                //口角を上げる（0-5が左端、0-1が右端）
                if(k == 5){
                    path.getCommands()[k].to.y = to.y - move.x;
                    path.getCommands()[k].cp2.y = cp2.y + move.x;
                }
                if(k == 1){
                    path.getCommands()[k].to.y = to.y - move.x;
                    path.getCommands()[k].cp2.y = cp2.y + move.x;
                }
                //下唇を開ける
                if(k == 6 || k == 0 || k == 1){
                    if(k != 1) path.getCommands()[k].to.y = to.y + move.x;
                    if(k == 6 || k == 1) path.getCommands()[k].cp1.y = cp1.y + move.x;
                    if(k == 0)path.getCommands()[k].cp2.y = cp2.y + move.x;
                }
                /*左右に歪ませる（2,3,4が上、6(0)が下）
                if(k <= 4 && k >= 2){
                    path.getCommands()[k].to.x = to.x + move2.x*2;
                    if(k != 2)path.getCommands()[k].cp1.x = cp1.x + move2.x*2;
                    path.getCommands()[k].cp2.x = cp2.x + move2.x*2;
                }
                if(k == 6 || k == 0){
                    path.getCommands()[k].to.x = to.x - move2.x*2;
                    if(k != 6)path.getCommands()[k].cp1.x = cp1.x - move2.x*2;
                    path.getCommands()[k].cp2.x = cp2.x - move2.x*2;
                }*/
            }
        }
        //描画
        path.draw();
    }
}
//--------------------------------------------------------------
//計算用関数　これらを使ってアニメーションします
//minとmaxの間を360フレームかけて周回カウントする式
float ofApp::rotateCount(float min, float max, float input){
    float rotation = abs( (1/PI) * ofWrapRadians(ofDegToRad(input)/(max/180)) );
    return min + max*rotation;
    //mapの方が後々わかりやすいかな？
    //return = ofMap(rotation, 0, 1, min, max);
}
//0起点でプラマイいくつの間を行ったり来たりする関数（2次曲線）（イーズインアウト）
ofVec2f ofApp::plusMinusEase(float length, float input){
    float ansX = length/2 * sin(ofDegToRad(input));
    float synchroY = length/2 * cos(ofDegToRad(input));
    return ofVec2f(ansX, synchroY);
}
//0起点でプラマイいくつの間を行ったり来たりする関数（正比例）（ライナー）
ofVec2f ofApp::plusMinusEven(float length, float input){
    float cramp = input/length - floor(input/length);
    if(cramp >= 0.5) cramp = 1-cramp;
    if(int(input/length)%2 == 0) cramp *= -1;
    float presentPoint =  cramp * length/2;
    float synchroY = length/2 - presentPoint;
    return ofVec2f(presentPoint, synchroY);
}

//minとmaxの間を行ったり来たりする関数（2次曲線）（イーズインアウト）
ofVec2f ofApp::PingPongEase(float min, float max, float input){
    float length = max - min;
    float ansX = min + length * abs(sin(ofDegToRad(input)));
    float synchroY = min + max - ansX;
    return ofVec2f(ansX, synchroY);
}
//minとmaxの間を行ったり来たりする関数（正比例）（ライナー）
ofVec2f ofApp::PingPongEven(float min, float max, float input){
    float length = max - min;
    //0〜totalの間の値を0〜1の比にまるめる//Unityだとclamp()という関数があるらしい
    //int%2で0か1かを求め、%1.0で小数点以下を求める
    float clamp = int(input/length)%2 - ((input/length)-int(input/length));
    //float clamp = abs(int(input/length)%2) - int(input/length)%1;
    
    //aとbの間の、比clampで指定される値を求める//UnityだとLerp()という関数があるらしい
    float presentPoint = min + length * clamp;
    float synchroY = max - presentPoint;
    return ofVec2f(presentPoint, synchroY);
}
//--------------------------------------------------------------
//サウンド変数確認用
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
//サウンド変数確認用
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

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    pose = !pose;
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

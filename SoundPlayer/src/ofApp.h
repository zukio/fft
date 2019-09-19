#pragma once

#include "ofMain.h"
#include "ofxAudioAnalyzer.h"
#include "ofSoundPlayerExtended.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
            void setupVisual();
            void setupAnalyzer();
		void update();
            void updateVisual();
            void updateAnalyzer();
		void draw();
            void drawVisual();
            void drawAnalyzer();
        //通常の関数
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
        //追加の関数 ------------------------------------------------
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void exit();
        //サウンド解析用
        void getAllValues();
        void drawDataView(); //確認用
        void drawWaveForms(); //確認用
        void updateWaveForms(float x, float y); //確認用
        //画像解析用（ピクセルの色・座標を解析）
        void getImgPos();
        //計算用（配列の順番をシャッフルするための関数）
        void shuffleID(int ArraySize);

    //--------------------------------------------------------------
        ofxAudioAnalyzer audioAnalyzer;
        ofSoundPlayerExtended player;
        ofSoundBuffer soundBuffer;

    //--------------------------------------------------------------
    int sampleRate;
    int bufferSize;
    
    float rms;
    float power;
    float volume;
    float pitchFreq;
    float pitchFreqNorm;
    float pitchConf;
    float pitchSalience;
    float hfc;
    float hfcNorm;
    float specComp;
    float specCompNorm;
    float centroid;
    float centroidNorm;
    float inharmonicity;
    float dissonance;
    float rollOff;
    float rollOffNorm;
    float oddToEven;
    float oddToEvenNorm;
    float strongPeak;
    float strongPeakNorm;
    float strongDecay;
    float strongDecayNorm;
    
    vector<float> spectrum;
    vector<float> melBands;
    vector<float> mfcc;
    vector<float> hpcp;
    vector<float> tristimulus;
    vector<float> spectrum_l, spectrum_r;
    
    //周波数のウェーブライン
    ofPolyline waveform_l, waveform_r;

    //ビート
    bool isOnset;
    //再生ボタン
    bool nowOnPlay;

    //--------------------------------------------------------------
    //アニメーションの滑らかさ
    float smoothing;
    //画像ファイル読み込み用
    ofImage baseImg;
    //描写座標
    vector<ofVec2f> drawPoint;
    vector<ofVec2f> specialPoint;
    vector<ofColor> colors;
    vector<int> shuffledNum;
    //演出用
    int spark;
    //画像と画面の縮尺を調整
    float reScale;
    //確認用
    ofVec2f position;
    bool dataView;

private:

};

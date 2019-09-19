#pragma once

#include "ofMain.h"
#include "ofxPDF.h"
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

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    //サウンド解析用 --------------------------------------------------
    ofxAudioAnalyzer audioAnalyzer;
    ofSoundPlayerExtended player;
    ofSoundBuffer soundBuffer;
    
    void exit();
    void getAllValues();
    void drawDataView(); //確認用
    void drawWaveForms(); //確認用
    void updateWaveForms(float x, float y); //確認用
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
    

    //PDF解析用 ------------------------------------------------
    void updatePDF(ofxPDF pdfData);
    void lipInner(ofxPDF pdfData);
    void lipBase(ofxPDF pdfData);
    void lipMask(ofxPDF pdfData);

    //PDF
    ofxPDF pdf_lipInner;
    ofxPDF pdf_lipMask;
    ofxPDF pdf_lipBase;
    //PDFをプレビューするためのキャッシュ
    ofFbo fbo_lipInner;
    ofFbo fbo_lipMask;
    ofFbo fbo_lipMaskC;
    ofFbo fbo_lipBase;
    //PDF元データ
    ofxPDF original_lipInner;
    ofxPDF original_lipMask;
    ofxPDF original_lipBase;
    //個別データ
    vector < vector<ofPoint> > access;
    vector<ofPolyline> outlines;
    
    //--------------------------------------------------------------
    //アニメーションの滑らかさ
    float smoothing;
    
    //演出用
    int spark;

    //カウント用
    int frameCount;
    int reFrameCount;
    float animPitch, ReAnimPitch;
    
    //計算用
    float rotateCount(float min, float max, float input);
    ofVec2f plusMinusEase(float length, float input);
    ofVec2f plusMinusEven(float length, float input);
    ofVec2f PingPongEase(float min, float max, float input);
    ofVec2f PingPongEven(float min, float max, float input);
    
    //確認用
    ofVec2f position;
    bool pose;
    bool dataView;

};

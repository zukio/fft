/* FFT解析 *************************************************** */
import ddf.minim.*;
import ddf.minim.analysis.*;

Minim minim;
AudioPlayer song;

//ビート解析用
BeatDetect beatFREQ;  //周波数エネルギー追跡
BeatDetect beatSOUND; //サウンドエネルギー追跡
BeatListener bl;

//周波数解析用
FFT fft;  
float specSize;  //周波数分解幅
float initBand;  //分解Hz

void setupMusic(){
  minim = new Minim(this);
  song = minim.loadFile(songName, 1024);
  fft  =  new  FFT(song.bufferSize(), song.sampleRate()) ;
  song.play();
  
  setupBeat();
  setupFFT();
}

// ★minimのBeatDetectクラスには2つのモード（サウンドエネルギー追跡と周波数エネルギー追跡）があります
void setupBeat(){
  // 1）周波数エネルギー追跡(FREQ_ENERGY mode)では周波数の異なる音（kick, hat snare）個別に追跡します
  // 周波数モードのインスタンスを生成（分析する音楽のサンプリングバッファサイズ、サンプルレート）
  beatFREQ = new BeatDetect(song.bufferSize(), song.sampleRate());
  // サンプリング感度 ＝ 待機時間 300 milliseconds
  beatFREQ.setSensitivity(300);  
  // 関数に渡す
  bl = new BeatListener(beatFREQ, song);
  
  // 2）サウンドエネルギー追跡ではをisOnset関数を使用してビートを取得します
  // サウンドモードのインスタンスを生成（引数なし、待機時間は自動的に10 milliseconds）
  beatSOUND = new BeatDetect();
}

void setupFFT(){
  //周波数分解幅を取得（全周波数を、この数の個数に分割して分析可能。通常は513）
  specSize = fft.specSize();
  
  //初期分解Hz(getBandWidth)を求める
  //例えば分解Hzが 43 なら、43 × 513 = 22059Hzまで解析可能
  initBand = fft.getBandWidth();
  
  //全周波数帯域を出力して、周波数がどうブロック分けされたかを確認
  //BandWidthにiを掛けると、それぞれ何番目のブロックに目当ての周波数が含まれるかが分かる。
  //for(int i = 0; i < fft.specSize(); i++) {
  //  println(i + " = " + fft.getBandWidth()*i + " ~ "+ fft.getBandWidth()*(i+1));
  //}
  
  //取得する周波数（平均値）の総数を描画するグリッドの数に揃える
  fft.linAverages( beatFREQ.detectSize()*4 );
}

void updateMusic(){
  //buffer内のサンプルを分析。累積データを分析するためフレームごとに呼び出す必要があります。
  beatSOUND.detect(song.mix);
  fft.forward(song.mix);
  
  volume = map(song.mix.level(), 0, 1, 1, max);
}

class BeatListener implements AudioListener{
  //minimのBeatDetectクラスによる拍検知アルゴリズム
  private BeatDetect beat;
  private AudioPlayer source;
  
  BeatListener(BeatDetect beat, AudioPlayer source){
    this.source = source;
    //新しいサンプリングバッファ
    this.source.addListener(this);
    this.beat = beat;
  }
  
  //mixは左右混合音
  void samples(float[] samps){
    beat.detect(source.mix);
  }
  
  void samples(float[] sampsL, float[] sampsR){
    beat.detect(source.mix);
  }
}

void stop(){
 song.close();
 minim.stop();
 super.stop();
 
}

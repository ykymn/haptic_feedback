# haptic_feedback

XIAO ESP32S3を利用してDCモーターを動かすプログラムです。\
UDP通信プロトコルを利用してXIAO ESP32S3の固定IPアドレスにデータを送信し、取得したデータに応じてモーターを動かします。\
頭の数字が大きいものが最新のものです。\
それ以前のものは動作テスト用のプログラムです。

使用する部品は以下の通りです。

ボード：          XIAO ESP32S3\
DCモーター：      GA12-N20\
バッテリー：      Lithium Ion Battery-3.7V-110 mAh\
ドライバーIC：    DRV8212DRLR\
昇圧コンバータ：  XCL103D543CR-G\
コンデンサ：      GRM188R6YA106MA73

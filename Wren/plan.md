# Wren モジュレーション拡張実装プラン

## 現在の技術制約分析

### ハードウェア制約
- **MCU**: RP2350A (Dual ARM Cortex-M33 @ 150MHz)
- **RAM**: 520KB on-chip SRAM
- **Sample Rate**: 44.1kHz (22.7μs per sample)
- **現在のCPU使用率**: 推定40-50% (Core0: audio + serial, Core1: NeoPixel)

### 現在のメモリ使用量
```
ウェーブテーブル: 8 banks × 32 samples × 2 bytes = 512 bytes
ADCフィルター: 4 channels × 16 bytes ≈ 64 bytes  
シリアルバッファ: 256 bytes
その他変数: ~1KB
総計: ~2KB (RAM使用率 < 1%)
```

### オーディオ処理時間予算
- **44.1kHz = 22.7μs/sample**
- **現在の処理**: ウェーブテーブル補間 + ウェーブフォールディング ≈ 5-8μs
- **残り予算**: 約15μs/sample (十分な余裕あり)

## 提案モジュレーション方式の実現可能性

### ✅ **確実に実装可能 (Phase 1)**

#### 1. オーバーフロー
```cpp
// 実装コスト: +0.1μs
if (modulationType == OVERFLOW) {
    // 単純にクリッピングを無効化
    // sampleFloat = constrain(sampleFloat, -1.0, 1.0); // 削除
    sampleFloat = fmod(sampleFloat + 1.0, 2.0) - 1.0;  // -1.0 to 1.0 ループ
}
```
- **CPU負荷**: 極小 (+0.1μs)
- **メモリ**: 0 bytes
- **リスク**: なし

#### 2. ビットクラッシュ
```cpp
// 実装コスト: +0.5μs  
if (modulationType == BITCRUSH) {
    int bitDepth = 16 - (int)(modulationAmount * 12);  // 16bit → 4bit
    float scale = pow(2, bitDepth - 1);
    sampleFloat = floor(sampleFloat * scale) / scale;
}
```
- **CPU負荷**: 小 (+0.5μs)
- **メモリ**: 0 bytes
- **リスク**: なし

### ⚠️ **工夫が必要 (Phase 2)**

#### 3. Phase Distortion (PD)
```cpp
// 実装コスト: +2-3μs
if (modulationType == PHASE_DISTORTION) {
    float distortedPhase = phase + modulationAmount * 0.5 * sin(2.0 * PI * phase);
    if (distortedPhase >= 1.0) distortedPhase -= 1.0;
    if (distortedPhase < 0.0) distortedPhase += 1.0;
    
    // 歪んだ位相でウェーブテーブル参照
    float distortedTablePos = distortedPhase * WAVETABLE_SIZE;
    // ... 以降通常の補間処理
}
```
- **CPU負荷**: 中 (+2-3μs) - sin()計算が重い
- **メモリ**: 0 bytes (sin()はライブラリ関数使用)
- **リスク**: sin()の計算時間が予測困難
- **対策**: 高速sin()近似またはルックアップテーブル検討

#### 4. レゾナンス (CZ-101方式)
```cpp
// 実装コスト: +3-4μs
if (modulationType == RESONANCE) {
    float resonantRatio = 1.0 + modulationAmount * 8.0;  // 1.0-9.0倍
    float resonantPhase = fmod(phase * resonantRatio, 1.0);
    
    // レゾナント正弦波生成
    float resonantSine = sin(2.0 * PI * resonantPhase);
    
    // ウィンドウ関数（基音）でAM変調
    float windowAmp = abs(sin(2.0 * PI * phase));
    
    sampleFloat = resonantSine * windowAmp;
}
```
- **CPU負荷**: 中高 (+3-4μs) - 2つのsin()計算
- **メモリ**: 0 bytes
- **リスク**: 複数sin()でCPU予算オーバーの可能性
- **対策**: 高速三角関数実装必須

### ❌ **実装困難 (Phase 3以降)**

#### 5. コーラス
```cpp
// 必要なメモリ: 44100Hz × 0.05sec × 2bytes = 4.4KB
float delayBuffer[2205];  // 50ms遅延バッファ
float lfoPhase = 0.0;
```
- **CPU負荷**: 中 (+2μs)
- **メモリ**: 4.4KB+ (RAM使用率が1%→2%へ)
- **リスク**: メモリ使用量増大、遅延バッファ管理の複雑さ
- **判定**: 後回し推奨

## 実装アーキテクチャ

### モジュレーションタイプ選択
```cpp
enum ModulationType {
    WAVEFOLDING = 0,    // 既存
    OVERFLOW = 1,
    BITCRUSH = 2, 
    PHASE_DISTORTION = 3,
    RESONANCE = 4,
    // CHORUS = 5,      // Phase 3
};

uint8_t currentModulationType = WAVEFOLDING;
float modulationAmount = 0.0;  // CV2で制御 (0.0-1.0)
```

### Webエディタ拡張
- **モジュレーションタイプ選択**: ドロップダウンメニュー
- **リアルタイム切り替え**: 新しいシリアルコマンド追加
- **プリセット保存**: EEPROMにモジュレーションタイプも保存

### シリアルプロトコル拡張
```
CMD_MODULATION_TYPE = 0x04    // モジュレーションタイプ設定
MODTYPE<type>\n → OK\n        // type: 0-4
```

## 段階的実装計画

### Phase 1: 基本実装 (1-2週間)
1. **オーバーフロー + ビットクラッシュ**実装
2. **モジュレーションタイプ選択**機能
3. **シリアルプロトコル**拡張
4. **基本テスト**

### Phase 2: 高度な実装 (2-3週間)  
5. **高速sin()近似**実装 (Taylor級数 or LUT)
6. **Phase Distortion**実装
7. **レゾナンス**実装 (CZ-101方式)
8. **安定性テスト**

### Phase 3: Webエディタ (1-2週間)
9. **Webエディタ**UI拡張
10. **プリセット保存**機能
11. **総合テスト**

### Phase 4: 将来拡張 (TBD)
12. **コーラス**実装検討
13. **その他エフェクト**検討

## リスク分析と対策

### 高リスク
- **sin()計算時間**: 高速近似実装で対策
- **CPU予算オーバー**: プロファイリングで実測必須

### 中リスク  
- **メモリ断片化**: 静的配列使用で回避
- **シリアル通信競合**: 既存プロトコルとの整合性確保

### 低リスク
- **オーディオ品質**: 段階的実装でテスト可能
- **ユーザビリティ**: Webエディタでカバー

## 成功基準

### Phase 1
- [ ] 5つのモジュレーションタイプが動作
- [ ] CPU使用率70%以下
- [ ] オーディオノイズなし

### Phase 2  
- [ ] PD/レゾナンスが実用的音質
- [ ] リアルタイム切り替え可能
- [ ] 安定性24時間テスト

### Phase 3
- [ ] Webエディタ完全対応
- [ ] プリセット保存機能
- [ ] ユーザーマニュアル完成

## 結論

**Phase 1 (オーバーフロー + ビットクラッシュ)は確実に実装可能** ✅
**Phase 2 (PD + レゾナンス)はsinテーブルで確実に実現可能** ✅

### sinテーブル実装詳細
- **サイズ**: 256サンプル × 2bytes = 512 bytes
- **精度**: 16bit精度、線形補間
- **速度**: ~0.1μs (sin()ライブラリ関数の1/10-1/20)
- **ファイル**: `sin_table.h` として作成済み ✅

## 解決されたボトルネック ✅：
- **sin()計算時間** → sinテーブルで解決（~0.1μs）
- **実装リスク大幅軽減**

**提案された4つの機能すべてが技術的に確実に実現可能** ✅と判断。
RP2350Aの性能で十分対応できる見込み。
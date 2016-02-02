#ifndef BallSprite_h
#define BallSprite_h

#include "cocos2d.h"

#define BALL_SIZE 106 // ボールのサイズ
#define ONE_ACTION_TIME 0.2 // ボール削除・落下アクションの時間

class BallSprite : public cocos2d::Sprite
{
public:
    // ボールの種類
    enum class BallType{
        Blue,
        Red,
        Green,
        Yellow,
        Purple,
        Pink,
    };
    
    // 位置インデックス
    struct PositionIndex{
        int _x;
        int _y;
        
        // コンストラクタ
        PositionIndex(){
            _x = 0;
            _y = 0;
        }
        
        PositionIndex(int x, int y){
            _x = x;
            _y = y;
        }
    };
    
    BallSprite(); // コンストラクタ
    // CREATE_FUNCを使用しない
    static BallSprite* create(BallType type, bool visible); // インスタンス生成
    virtual bool init(BallType type, bool visible); // 初期化
    
    // マクロを利用したアクセッサ
    CC_SYNTHESIZE(int, _removeNo, RemoveNo); // ボール削除の順番
    CC_SYNTHESIZE(bool, _checkedX, CheckedX); // 横方向確認フラグ
    CC_SYNTHESIZE(bool, _checkedY, CheckedY); // 縦方向確認フラグ
    CC_SYNTHESIZE(int, _fallCount, FallCount); // ボール落下時の落下段数
    CC_SYNTHESIZE_READONLY(BallType, _ballType, BallType); // ボールの種類
    CC_PROPERTY(PositionIndex, _positionIndex, PositionIndex); // 位置インデックス
    
    void setPositionIndexAndChangePosition(PositionIndex positionIndex);
    void resetParams(); // パラメータのリセット
    void resetPosition(); // 初期位置へ移動
    void removingAndFallngAnimation(int maxRemoveNo); // 消去アニメーションと落下アニメーション
    
    static std::string getBallImageFilePath(BallType type); // ボール画像パス取得
    static cocos2d::Point getPositionForPositionIndex(PositionIndex positionIndex); // 位置インデックスからPoint取得
    static int generateTag(PositionIndex positionIndex); // 位置インデックスからタグを取得
    
protected:
    void removingAnimation(int maxRemoveNo); // ボール削除アニメーション
    void fallingAnimation(int maxRemoveNo); // ボール落下アニメーション
};

#endif /* BallSprite_h */

#ifndef GameLayer_h
#define GameLayer_h

#include "cocos2d.h"
#include <random>
#include "BallSprite.h"

class GameLayer : public cocos2d::Layer
{
protected:
    // ボールのチェック方向
    enum class Direction{
        _x,
        _y,
    };
    
    // Zオーダー
    enum ZOrder{
        BgForCharacter = 0,
        BgForPuzzle = 1,
        Enemy = 2,
        EnemyHp = 3,
        Char = 4,
        CharHp = 5,
        Ball = 6,
        Level = 7,
        Result = 8,
    };
    
    std::default_random_engine _engine; // 乱数生成エンジン
    std::discrete_distribution<int> _distForBall; // 乱数の分布
    
    void initBackGround(); // 背景の初期化
    void initBalls(); // ボールの初期表示
    BallSprite* newBall(BallSprite::PositionIndex positionIndex); // 新規ボール作成
    
public:
    GameLayer();
    virtual bool init(); // 初期化
    CREATE_FUNC(GameLayer); // crete関数作成
    // GameLayerクラスのインスタンスを持つシーンを生成する
    static cocos2d::Scene* createScene(); // シーン生成
};

#endif /* GameLayer_h */

#include "BallSprite.h"

USING_NS_CC;

// コンストラクタ
BallSprite::BallSprite()
: _removeNo(0)
, _checkedX(false)
, _checkedY(false)
, _fallCount(0)
, _positionIndex(0, 0)
{
}

// インスタンス生成
BallSprite* BallSprite::create(BallType type, bool visible)
{
    BallSprite* pRet = new BallSprite();
    if(pRet && pRet->init(type, visible)){
        pRet->autorelease();
        return pRet;
    }else{
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

// 初期化
bool BallSprite::init(BallType type, bool visible)
{
    if(!Sprite::initWithFile(getBallImageFilePath(type))){
        return false;
    }
    
    _ballType = type;
    setVisible(visible);
    return true;
}

// パラメータのリセット
void BallSprite::resetParams()
{
    _removeNo = 0;
    _checkedX = false;
    _checkedY = false;
    _fallCount = 0;
}

// 初期位置へ移動
void BallSprite::resetPosition()
{
    // 位置を変更する
    setPosition(getPositionForPositionIndex(_positionIndex));
}

// 位置インデックスを返す
BallSprite::PositionIndex BallSprite::getPositionIndex()
{
    // 位置インデックスを返す
    return _positionIndex;
}

// 位置インデックスとタグを変更する
void BallSprite::setPositionIndex(PositionIndex positionIndex)
{
    // 位置インデックスを保存する
    _positionIndex = positionIndex;
    
    // タグをセットする
    setTag(generateTag(_positionIndex));
}

// インデックスとタグと位置を変更する
void BallSprite::setPositionIndexAndChangePosition(PositionIndex positionIndex)
{
    // インデックスとタグを変更する
    setPositionIndex(positionIndex);
    
    // 位置を変更する
    resetPosition();
}

// 消去アニメーションと落下アニメーション
void BallSprite::removingAndFallngAnimation(int maxRemoveNo)
{
    // ボールを消すアニメーション
    removingAnimation(maxRemoveNo);
    
    // ボールを落とすアニメーション
    fallingAnimation(maxRemoveNo);
}

// ボール削除アニメーション
void BallSprite::removingAnimation(int maxRemoveNo)
{
    if(_removeNo > 0){
        // ボールを消すアニメーション
        auto delay1 = DelayTime::create(ONE_ACTION_TIME * (_removeNo - 1));
        auto fade = FadeTo::create(ONE_ACTION_TIME, 0);
        auto delay2 = DelayTime::create(ONE_ACTION_TIME * (maxRemoveNo - _removeNo));
        auto removeSelf = RemoveSelf::create(false);
        
        // アニメーション実行
        runAction(Sequence::create(delay1, fade, delay2, removeSelf, NULL));
    }
}

// ボール落下アニメーション
void BallSprite::fallingAnimation(int maxRemoveNo)
{
    if(_fallCount > 0){
        // ボールを落とすアニメーション
        setPositionIndex(PositionIndex(_positionIndex._x, _positionIndex._y - _fallCount));
        
        auto delay = DelayTime::create(ONE_ACTION_TIME * maxRemoveNo);
        auto show = Show::create();
        auto move = MoveTo::create(ONE_ACTION_TIME, getPositionForPositionIndex(getPositionIndex()));
        
        // アニメーション実行
        runAction(Sequence::create(delay, show, move, nullptr));
    }
}

// ボール画像の取得
std::string BallSprite::getBallImageFilePath(BallType type)
{
    // ボールタイプに適した画像を返す
    switch (type) {
        case BallType::Red:
            return "red.png";
        case BallType::Blue:
            return "blue.png";
        case BallType::Yellow:
            return "yellow.png";
        case BallType::Green:
            return "green.png";
        case BallType::Purple:
            return "purple.png";
        default:
            return "pink.png";
    }
}

// 位置インデックスからPoint取得
Point BallSprite::getPositionForPositionIndex(PositionIndex positionIndex)
{
    return Point(BALL_SIZE * (positionIndex._x - 0.5) + 1, BALL_SIZE * (positionIndex._y - 0.5) + 1);
}

// 位置インデックスからタグを取得
int BallSprite::generateTag(PositionIndex positionIndex)
{
    return positionIndex._x * 10 + positionIndex._y;
}
#include "Character.h"

USING_NS_CC;

// コンストラクタ
Character::Character()
:_hp(0)
,_maxHP(0)
,_attack(0)
,_element(Element::None)
,_turnCount(0)
,_remaingTurn(0)
{
    
}

Character* Character::create()
{
    Character *pRet = new Character();
    pRet->autorelease();
    
    return pRet;
}

int Character::getTurnCount()
{
    return _turnCount;
}

void Character::setTurnCount(int turnCount)
{
    _turnCount = turnCount;
    _remaingTurn = _turnCount;
}

float Character::getHpPercentage()
{
    return _hp * 100 / _maxHP;
}

bool Character::isAttackTurn()
{
    _remaingTurn--;
    
    if(_remaingTurn <= 0){
        _remaingTurn = _turnCount;
        return true;
    }
    return false;
}

int Character::getDamage(int ballCount, int chainCount, Character *attacker, Character *defender)
{
    float baseDamage = ballCount / 3.0 * 100;
    float chainBonus = powf(1.1, chainCount - 1);
    
    float elementBonus = getElementBonus(attacker->getElement(), defender->getElement());
    return baseDamage * chainBonus * elementBonus;
}

float Character::getElementBonus(Element attackElement, Element defenderElement)
{
    switch (attackElement) {
        case Element::Fire:
            switch (defenderElement) {
                case Element::Wind: return 2;
                case Element::Water: return 0.5;
                default: return 1;
            }
        case Element::Water:
            switch (defenderElement) {
                case Element::Fire: return 2;
                case Element::Wind: return 0.5;
                default: return 1;
            }
        case Element::Wind:
            switch (defenderElement) {
                case Element::Water: return 2;
                case Element::Fire: return 0.5;
                default: return 1;
            }
        case Element::Holy:
            switch (defenderElement) {
                case Element::Shadow: return 2;
                default: return 1;
            }
        case Element::Shadow:
            switch (defenderElement) {
                case Element::Holy: return 2;
                default: return 1;
            }
        default:
            return 1;
    }
}


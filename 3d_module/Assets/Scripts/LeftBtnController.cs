using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class LeftBtnController : MonoBehaviour, IPointerClickHandler
{
    public void OnPointerClick(PointerEventData pointerEventData)
    {
        if (pointerEventData.button == PointerEventData.InputButton.Left)
            CubeController.turnLeftFace(90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Right)
            CubeController.turnLeftFace(-90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Middle)
            CubeController.turnLeftFace(180.0f);
    }
}

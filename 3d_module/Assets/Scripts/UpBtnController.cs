using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class UpBtnController : MonoBehaviour, IPointerClickHandler
{
    public void OnPointerClick(PointerEventData pointerEventData)
    {
        if (pointerEventData.button == PointerEventData.InputButton.Left)
            CubeController.turnUpFace(90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Right)
            CubeController.turnUpFace(-90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Middle)
            CubeController.turnUpFace(180.0f);
    }
}

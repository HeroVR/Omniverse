using UnityEngine;
using System.Collections;
using UnityEngine.EventSystems;
public class OVUIEventListener : MonoBehaviour
{
    public delegate void VoidDelegate(GameObject go);
    public delegate void VectorDelegate(GameObject go, PointerEventData eventData);
    public VoidDelegate onClick;
    public VoidDelegate onDown;
    public VoidDelegate onEnter;
    public VoidDelegate onExit;
    public VoidDelegate onUp;
    public VoidDelegate onSelect;
    public VoidDelegate onUpdateSelect;
    public VoidDelegate onDeSelect;
    public VectorDelegate onDrag;
    public VoidDelegate onDragEnd;
    public VoidDelegate onDrop;
    public VoidDelegate onScroll;
    public VoidDelegate onMove;
    public VectorDelegate onHover;


    public object parameter;

    public void OnPointerClick(PointerEventData eventData) { if (onClick != null) onClick(gameObject); }
    public void OnPointerDown(PointerEventData eventData) { if (onDown != null) onDown(gameObject); }
    public void OnPointerEnter(PointerEventData eventData) { if (onEnter != null) onEnter(gameObject); }
    public void OnPointerExit(PointerEventData eventData) { if (onExit != null) onExit(gameObject); }
    public void OnPointerUp(PointerEventData eventData) { if (onUp != null) onUp(gameObject); }
    public void OnSelect(BaseEventData eventData) { if (onSelect != null) onSelect(gameObject); }
    public void OnUpdateSelected(BaseEventData eventData) { if (onUpdateSelect != null) onUpdateSelect(gameObject); }
    public void OnDeselect(BaseEventData eventData) { if (onDeSelect != null) onDeSelect(gameObject); }
    public void OnDrag(PointerEventData eventData) { if (onDrag != null) onDrag(gameObject, eventData); }
    public void OnEndDrag(PointerEventData eventData) { if (onDragEnd != null) onDragEnd(gameObject); }
    public void OnDrop(PointerEventData eventData) { if (onDrop != null) onDrop(gameObject); }
    public void OnScroll(PointerEventData eventData) { if (onScroll != null) onScroll(gameObject); }
    public void OnMove(AxisEventData eventData) { if (onMove != null) onMove(gameObject); }
    public void OnHover(PointerEventData eventData) { if (onHover != null) onHover(gameObject, eventData); }

    static public OVUIEventListener Get(GameObject go)
    {
        OVUIEventListener listener = go.GetComponent<OVUIEventListener>();
        if (listener == null) listener = go.AddComponent<OVUIEventListener>();
        return listener;
    }
    //static public OVUIEventListener Get(GameObject go, PointerEventData eventData)
    //{
    //    OVUIEventListener listener = go.GetComponent<OVUIEventListener>();
    //    if (listener == null) listener = go.AddComponent<OVUIEventListener>();
    //    return listener;
    //}
    static public OVUIEventListener Get(Transform transform)
    {
        OVUIEventListener listener = transform.GetComponent<OVUIEventListener>();
        if (listener == null) listener = transform.gameObject.AddComponent<OVUIEventListener>();
        return listener;
    }
}
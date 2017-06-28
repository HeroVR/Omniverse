using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using UnityEditor.UI;
// Custom Editor the "old" way by modifying the script variables directly.
// No handling of multi-object editing, undo, and prefab overrides!
[CanEditMultipleObjects, CustomEditor(typeof(OVButton), true)]
public class OVButtonEditor : SelectableEditor
{
    private SerializedProperty m_OnClickProperty;

    private SerializedProperty _StyleNormal;
    private SerializedProperty _StyleHover;
    private SerializedProperty _StylePress;
    private SerializedProperty _StyleDisable;
    private SerializedProperty _StylePressed;
    private SerializedProperty _isCheckBox;
    private SerializedProperty _CheckGameObject;
    private SerializedProperty _Checknormal;
    private SerializedProperty _Buttontext;
    //private SerializedProperty _isSlider;

    [MenuItem("Examples/Editor GUILayout Toggle Usage")]
    protected override void OnEnable()
    {
        //    this._isCheckBox = base.serializedObject.FindProperty("_isCheckBox");
        //this._isSlider = base.serializedObject.FindProperty("_isSlider");
        this.m_OnClickProperty = base.serializedObject.FindProperty("m_OnClick");
        this._StyleNormal = base.serializedObject.FindProperty("_StyleNormal");
        this._StyleHover = base.serializedObject.FindProperty("_StyleHover");
        this._StylePress = base.serializedObject.FindProperty("_StylePress");
        this._StyleDisable = base.serializedObject.FindProperty("_StyleDisable");
        this._StylePressed = base.serializedObject.FindProperty("_StylePressed");
        this._CheckGameObject = base.serializedObject.FindProperty("_CheckGameObject");
        this._Checknormal = base.serializedObject.FindProperty("_Checknormal");
        this._Buttontext = base.serializedObject.FindProperty("_Buttontext");
    }

    public override void OnInspectorGUI()
    {
		OVButton hb = (OVButton)target;
        hb._isSlider = (bool)EditorGUILayout.Toggle("_isSlider", hb._isSlider);
        hb._isCheckBox = (bool)EditorGUILayout.Toggle("_isCheckBox", hb._isCheckBox);
        if(hb._isCheckBox)
        {
            EditorGUILayout.PropertyField(this._CheckGameObject, new GUILayoutOption[0]);
            base.serializedObject.ApplyModifiedProperties();
            EditorGUILayout.PropertyField(this._Checknormal, new GUILayoutOption[0]);
            base.serializedObject.ApplyModifiedProperties();
        }
        //hb._StyleNormal = (Sprite)EditorGUILayout.ObjectField("_StyleNormal", hb._StyleNormal, typeof(Sprite), true, new GUILayoutOption[0]);
        //hb._StyleHover = (Sprite)EditorGUILayout.ObjectField("_StyleHover", hb._StyleHover, typeof(Sprite), true, new GUILayoutOption[0]);
        //hb._StylePress = (Sprite)EditorGUILayout.ObjectField("_StylePress", hb._StylePress, typeof(Sprite), true, new GUILayoutOption[0]);
        //hb._StyleDisable = (Sprite)EditorGUILayout.ObjectField("_StyleDisable", hb._StyleDisable, typeof(Sprite), true, new GUILayoutOption[0]);
        //hb._StylePressed = (Sprite)EditorGUILayout.ObjectField("_StylePressed", hb._StylePressed, typeof(Sprite), true, new GUILayoutOption[0]);
        EditorGUILayout.Space();//空行

        //    EditorGUILayout.PropertyField(this._isCheckBox, new GUILayoutOption[0]);
        EditorGUILayout.PropertyField(this._Buttontext, new GUILayoutOption[0]);
        base.serializedObject.ApplyModifiedProperties();
        EditorGUILayout.PropertyField(this._StyleNormal, new GUILayoutOption[0]);
        base.serializedObject.ApplyModifiedProperties();
        EditorGUILayout.PropertyField(this._StyleHover, new GUILayoutOption[0]);
        base.serializedObject.ApplyModifiedProperties();
        EditorGUILayout.PropertyField(this._StylePress, new GUILayoutOption[0]);
        base.serializedObject.ApplyModifiedProperties();
        EditorGUILayout.PropertyField(this._StyleDisable, new GUILayoutOption[0]);
        base.serializedObject.ApplyModifiedProperties();
        EditorGUILayout.PropertyField(this._StylePressed, new GUILayoutOption[0]);
        base.serializedObject.ApplyModifiedProperties();
        // EditorGUILayout.PropertyField(this.m_InteractableProperty, new GUILayoutOption[0]);
        EditorGUILayout.Space();//空行
        base.serializedObject.Update();
        EditorGUILayout.PropertyField(this.m_OnClickProperty, new GUILayoutOption[0]);
        base.serializedObject.ApplyModifiedProperties();
    }
}

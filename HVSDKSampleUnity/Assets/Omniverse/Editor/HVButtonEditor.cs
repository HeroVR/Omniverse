using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using UnityEditor.UI;
// Custom Editor the "old" way by modifying the script variables directly.
// No handling of multi-object editing, undo, and prefab overrides!
[CanEditMultipleObjects, CustomEditor(typeof(HVButton), true)]
public class HVButtonEditor : SelectableEditor
{
    private SerializedProperty m_OnClickProperty;

    private SerializedProperty _StyleNormal;
    private SerializedProperty _StyleHover;
    private SerializedProperty _StylePress;
    private SerializedProperty _StyleDisable;
    private SerializedProperty _StylePressed;
    private SerializedProperty _isCheckBox;

    [MenuItem("Examples/Editor GUILayout Toggle Usage")]
    protected override void OnEnable()
    {
    //    this._isCheckBox = base.serializedObject.FindProperty("_isCheckBox");
        this.m_OnClickProperty = base.serializedObject.FindProperty("m_OnClick");
        this._StyleNormal = base.serializedObject.FindProperty("_StyleNormal");
        this._StyleHover = base.serializedObject.FindProperty("_StyleHover");
        this._StylePress = base.serializedObject.FindProperty("_StylePress");
        this._StyleDisable = base.serializedObject.FindProperty("_StyleDisable");
        this._StylePressed = base.serializedObject.FindProperty("_StylePressed");
    }

    public override void OnInspectorGUI()
    {
        HVButton hb = (HVButton)target;
        hb._isCheckBox = (bool)EditorGUILayout.Toggle("_isCheckBox", hb._isCheckBox);
        //hb._StyleNormal = (Sprite)EditorGUILayout.ObjectField("_StyleNormal", hb._StyleNormal, typeof(Sprite), true, new GUILayoutOption[0]);
        //hb._StyleHover = (Sprite)EditorGUILayout.ObjectField("_StyleHover", hb._StyleHover, typeof(Sprite), true, new GUILayoutOption[0]);
        //hb._StylePress = (Sprite)EditorGUILayout.ObjectField("_StylePress", hb._StylePress, typeof(Sprite), true, new GUILayoutOption[0]);
        //hb._StyleDisable = (Sprite)EditorGUILayout.ObjectField("_StyleDisable", hb._StyleDisable, typeof(Sprite), true, new GUILayoutOption[0]);
        //hb._StylePressed = (Sprite)EditorGUILayout.ObjectField("_StylePressed", hb._StylePressed, typeof(Sprite), true, new GUILayoutOption[0]);


    //    EditorGUILayout.PropertyField(this._isCheckBox, new GUILayoutOption[0]);
        EditorGUILayout.PropertyField(this._StyleNormal, new GUILayoutOption[0]);
        EditorGUILayout.PropertyField(this._StyleHover, new GUILayoutOption[0]);
        EditorGUILayout.PropertyField(this._StylePress, new GUILayoutOption[0]);
        EditorGUILayout.PropertyField(this._StyleDisable, new GUILayoutOption[0]);
        EditorGUILayout.PropertyField(this._StylePressed, new GUILayoutOption[0]);
        // EditorGUILayout.PropertyField(this.m_InteractableProperty, new GUILayoutOption[0]);
        EditorGUILayout.Space();//空行
        base.serializedObject.Update();
        EditorGUILayout.PropertyField(this.m_OnClickProperty, new GUILayoutOption[0]);
        base.serializedObject.ApplyModifiedProperties();
    }
}

import React from 'react'
import { findNodeHandle } from 'react-native/types'
import { FilamentProxy } from './FilamentProxy'
import { NativeFilamentView, NativeProps } from './FilamentViewNativeComponent'

type FilamentViewProps = NativeProps

type RefType = typeof NativeFilamentView

export class FilamentView extends React.PureComponent<FilamentViewProps> {
  private readonly ref: React.RefObject<RefType>

  constructor(props: FilamentViewProps) {
    super(props)
    this.ref = React.createRef<RefType>()
  }

  private get handle(): number {
    const nodeHandle = findNodeHandle(this.ref.current)
    if (nodeHandle == null || nodeHandle === -1) {
      throw new Error("Could not get the FilamentView's native view tag! Does the FilamentView exist in the native view-tree?")
    }

    return nodeHandle
  }

  private getSurfaceInternal() {
    FilamentProxy.getSurface()
  }

  /** @internal */
  public render(): React.ReactNode {
    return <NativeFilamentView {...this.props} />
  }
}